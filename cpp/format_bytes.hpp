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
 *   void format_bytes(std::basic_string<CharT>& repr
 *                   , ByteT const bytes
 *                   , std::size_t const decimal=2u
 *                   , std::size_t const reduced_unit=1024u);
 * (2)
 *   template<typename CharT, typename ByteT>
 *   void format_bytes(std::basic_string<CharT>& repr
 *                   , ByteT const bytes
 *                   , std::basic_string<CharT> const& indicator
 *                   , std::size_t const decimal=2u
 *                   , std::size_t const reduced_unit=1024u);
 * (3)
 *   template<typename CharT, typename ByteT, typename InputIt>
 *   void format_bytes(std::basic_string<CharT>& repr
 *                   , ByteT const bytes
 *                   , InputIt first, InputIt last
 *                   , std::size_t const decimal=2u
 *                   , std::size_t const reduced_unit=1024u);
 * (4)
 *   template<typename CharT, typename ByteT, typename InputIt>
 *   void format_bytes(std::basic_string<CharT>& repr
 *                   , ByteT const bytes
 *                   , InputIt first, InputIt last
 *                   , std::basic_string<CharT> const& indicator
 *                   , std::size_t const decimal=2u
 *                   , std::size_t const reduced_unit=1024u);
 *
 * [Usage]
 *
 *   using namespace ymh::misc;
 *
 *   std::string s;
 *   format_bytes(s, 18446640);
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

namespace ymh
{
namespace misc
{
    
template <typename CharT> 
struct Indicators;

template <>
struct Indicators<char> final
{
    static char const* const value[];
    static std::size_t const size;
};
char const* const Indicators<char>::value[]
= { "Bytes", "KB", "MB", "GB", "TB" };
std::size_t const Indicators<char>::size = sizeof(value) / sizeof(value[0]);

template <>
struct Indicators<wchar_t> final
{
    static wchar_t const* value[];
    static std::size_t const size;
};
wchar_t const* Indicators<wchar_t>::value[]
= { L"Bytes", L"KB", L"MB", L"GB", L"TB" };
std::size_t const Indicators<wchar_t>::size = sizeof(value) / sizeof(value[0]);

}  // namespace misc
}  // namespace ymh

#endif  // YMH_MISC_FORMAT_BYTES_HPP

