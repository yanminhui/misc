/*
 * ============
 * FORMAT_BYTES
 * ============ https://github.com/yanminhui/misc
 *
 * Licensed under the MIT License <http:*opensource.org/licenses/MIT>.
 * Copyright (c) 2019 yanminhui <mailto:yanminhui163@163.com>.
 *
 * Given a byte count, converts it to human-readable format 
 * and returns a string consisting of a value and a units indicator.
 *
 * Depending on the size of the value, the units part is bytes, 
 * KB (kibibytes), MB (mebibytes), GB (gibibytes), TB (tebibytes), 
 * or PB (pebibytes)...
 *
 * [Function Prototype]
 *
 * (1)
 *   template<typename CharT, typename ByteT>
 *   CharT const* format_bytes(std::basic_string<CharT>& repr
 *                           , ByteT const bytes
 *                           , std::size_t const decimal=2u
 *                           , std::size_t const reduced_unit=1024u);
 * (2)
 *   template<typename CharT, typename ByteT, typename IndicatorT>
 *   CharT const* format_bytes(std::basic_string<CharT>& repr
 *                           , ByteT const bytes
 *                           , IndicatorT&& indicator
 *                           , std::size_t const decimal=2u
 *                           , std::size_t const reduced_unit=1024u);
 * (3)
 *   template<typename CharT, typename ByteT, typename InputIt>
 *   CharT const* format_bytes(std::basic_string<CharT>& repr
 *                           , ByteT const bytes
 *                           , InputIt first, InputIt last
 *                           , std::size_t const decimal=2u
 *                           , std::size_t const reduced_unit=1024u);
 * (4)
 *   template<typename CharT, typename ByteT
 *          , typename InputIt, typename IndicatorT>
 *   CharT const* format_bytes(std::basic_string<CharT>& repr
 *                           , ByteT const bytes
 *                           , InputIt first, InputIt last
 *                           , IndicatorT&& indicator
 *                           , std::size_t const decimal=2u
 *                           , std::size_t const reduced_unit=1024u);
 *
 * [Usage]
 *
 *   using namespace ymh::misc;
 *
 *   std::string s;
 *   std::cout << format_bytes(s, 18446640) << std::endl;
 *
 *   // equal to:
 *   //
 *   // auto indicators = { "Bytes", "KB", "MB", "GB" };
 *   // format_bytes(s, 18446640
 *   //            , std::begin(indicators), std::end(indicators)
 *   //            , "MB", 2u, 1024u);
 *
 * [Output]
 *
 *   17.60 MB
 *
 */

#ifndef YMH_MISC_FORMAT_BYTES_HPP
#define YMH_MISC_FORMAT_BYTES_HPP

#include <cmath>
#include <iomanip>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <system_error>
#include <type_traits>

namespace ymh
{
namespace misc
{

template<typename CharT, typename ByteT
       , typename InputIt, typename IndicatorT>
CharT const* format_bytes(std::basic_string<CharT>& repr
                        , ByteT const bytes
                        , InputIt first, InputIt last
                        , IndicatorT&& indicator
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u)
{
    constexpr auto before_begin_step = -1; 
    if (first == last)
    {
        throw std::system_error(std::make_error_code
                (std::errc::invalid_argument)
            , "[first, last) is invalid range in format_bytes");
    }

    // `indicator` may be 
    // `CharT const*`, `CharT const[]` or `std::basic_string<CharT>`
    std::basic_string<CharT> indicator_s(indicator);
    
    // try get `indicator_step` and `indicator_s`
    auto indicator_step = before_begin_step;
    if (indicator_s.empty())
    {
        indicator_s = *first;
    }
    else
    {
        for (auto it = first; it != last; ++it)
        {
            if (*it == indicator_s)
            {
                break;
            }
            ++indicator_step;
        }
    }

    // represent
    std::basic_ostringstream<CharT> oss;
    oss << std::fixed << std::setprecision(decimal);
    
    // calc `value`
    if (0 < bytes)
    {	
        if (indicator_step == before_begin_step)
        // try calc `indicator_step` and `indicator_s`
        {
            auto t = std::floor(std::log(bytes) / std::log(reduced_unit));
            indicator_step = static_cast<decltype(indicator_step)>(t);
            if (indicator_step <= before_begin_step)
            {
                indicator_step = before_begin_step + 1;
            }
            else if (std::distance(first, last) <= indicator_step)
            {
                indicator_step = std::distance(first, last) - 1;   
            }

            std::advance(first, indicator_step);
            indicator_s = *first;
        }
        oss << (bytes / (std::pow(reduced_unit, indicator_step) * 1.0));
    }
    else
    {
        oss << bytes;
    }

    oss << oss.widen(' ') << indicator_s;
    repr = oss.str();
    return repr.c_str();
}

template<typename CharT, typename ByteT, typename InputIt>
CharT const* format_bytes(std::basic_string<CharT>& repr
                        , ByteT const bytes
                        , InputIt first, InputIt last
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u)
{
    typename std::remove_reference<decltype(repr)>::type indicator;
    return format_bytes(repr, bytes, first, last, indicator
                      , decimal, reduced_unit);
}

template<typename CharT, typename ByteT, typename IndicatorT>
CharT const* format_bytes(std::basic_string<CharT>& repr
                        , ByteT const bytes
                        , IndicatorT&& indicator
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u)
{ 
    struct Indicators
    {
        static constexpr std::initializer_list<char const*> get(char) 
        {
            return {"Bytes", "KB", "MB", "GB"
                  , "TB", "PB", "EB", "ZB", "YB"};
        }
        static constexpr std::initializer_list<wchar_t const*> get(wchar_t)
        {
            return {L"Bytes", L"KB", L"MB", L"GB"
                  , L"TB", L"PB", L"EB", L"ZB", L"YB"};
        }
    };
    
    constexpr auto v = Indicators::get(CharT());
    return format_bytes(repr, bytes, std::begin(v), std::end(v)
                      , std::forward<IndicatorT&&>(indicator)
                      , decimal, reduced_unit);
}

template<typename CharT, typename ByteT>
CharT const* format_bytes(std::basic_string<CharT>& repr
                        , ByteT const bytes
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u)
{
    typename std::remove_reference<decltype(repr)>::type indicator;
    return format_bytes(repr, bytes, indicator
                      , decimal, reduced_unit);
}

}  // namespace misc
}  // namespace ymh

#endif  // YMH_MISC_FORMAT_BYTES_HPP

