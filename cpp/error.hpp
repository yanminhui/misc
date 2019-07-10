/*
 * ==============================
 * ERROR: Error storage structure
 * ============================== https://github.com/yanminhui/misc
 *
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 * Copyright (c) 2019 yanminhui <mailto:yanminhui163@163.com>.
 *
 * Save std::error_code, boost.system, GetLastError(), user custom error code 
 * and error message to class [w]error_t.
 *
 * Query error information by dump() or dump_backtrace() from class [w]error_t,
 * it can query error domain, value, message by the numbers of [w]error_t.
 *
 * [Function Prototype]
 *
 * (1) User Custom Error
 *    
 *    SET_ERROR_CUSTOM[W](error_t&, domain, value, format_string, ...);
 *    SET_ERROR_MESSAGE[W](error_t&, value, format_string, ...);
 *    SET_ERROR_STRING[W](error_t&, format_string, ...);
 *
 * (2) std::error_code or boost.system
 *
 *    SET_ERROR_CODE[W](error_t&, error_code);
 *    MAKE_ERROR_CODE[W](error_t&, errc_t);
 *
 * (3) System Error
 *   
 *    SET_SYSTEM_ERROR[W](error_t&, ::GetLastError());  // errno
 *
 * (4) Catch Exception
 *
 *    ERROR_TRY[W] {
 *      // throw exception
 *    } ERROR_CATCH[W](error_t&)
 *
 * (5) Print Error Message to Stream
 *
 *    void error_t::dump(std::basic_ostream<charT>&);
 *    void error_t::dump_backtrace(std::basic_ostream<charT>&);
 *
 * [Usage]
 *
 *   using namespace ymh;
 *
 *   error_t err;
 *   SET_ERROR_STRING(err, "Open file %s failed", "error.log");
 *   
 *   if (err)  // return true if error occur
 *   {
 *      SET_SYSTEM_ERROR(err, ::GetLastError());
 *   }
 *
 * [Output]
 *
 *   File "xxx.cpp", line ?, in <function>: Open file error.log failed
 *   File "xxx.cpp", line ?, in <function>: Success
 *
 */

#ifndef YMH_ERROR_HPP
#define YMH_ERROR_HPP

#include <codecvt>
#include <limits>
#include <locale>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

/*
#if defined(_MSVC_VER) && defined(_WIN32)
#   include <filesystem>
#else
#   include <experimental/filesystem>
#endif
*/

#if defined(BOOST_SYSTEM_ERROR_CODE_HPP) && !defined(YMH_ERR_WITH_BOOST_SYSTEM)
#   define YMH_ERR_WITH_BOOST_SYSTEM 1
#endif  // BOOST_SYSTEM_ERROR_CODE_HPP && !YMH_ERR_WITH_BOOST_SYSTEM

#if defined(YMH_ERR_WITH_BOOST_SYSTEM)
#   include <boost/system/error_code.hpp>
#endif  // YMH_ERR_WITH_BOOST_SYSTEM

/* User custom error message
 */
#define SET_ERROR_CUSTOM(err, domain, val, fmt, ...) \
    do { err.set_error_custom2(__FILE__, __LINE__, __FUNCTION__ \
        , domain, val, fmt, ##__VA_ARGS__); } while (0)

/* User custom error message, except domain
 */
#define SET_ERROR_MESSAGE(err, val, fmt, ...) \
    do { err.set_error_message2(__FILE__, __LINE__, __FUNCTION__ \
        , val, fmt, ##__VA_ARGS__); } while (0)

#define SET_ERROR_STRING(err, fmt, ...) \
    do { err.set_error_string2(__FILE__, __LINE__, __FUNCTION__\
         , fmt, ##__VA_ARGS__); } while (0)

/* std::error_code or boost.system
 */
#define SET_ERROR_CODE(err, ec) \
    do { err.set_error_code2(__FILE__, __LINE__, __FUNCTION__, ec); } while (0)

#define MAKE_ERROR_CODE(err, e) \
    do { err.make_error_code2(__FILE__, __LINE__, __FUNCTION__, e); } while (0)

/* GetLastError()
 */
#define SET_SYSTEM_ERROR(err, val) \
    do { err.set_system_error2(__FILE__, __LINE__, __FUNCTION__, val); } while (0)

/* std::exception
 */
#define ERROR_TRY try

#define ERROR_CATCH_BOOST_SYSTEM(err) \
    catch (boost::system::system_error const& _error) { \
        SET_ERROR_CUSTOM(err \
            , _error.code().category().name() \
            , _error.code().value(), _error.what()); } 

#define ERROR_CATCH_STD_SYSTEM(err) \
    catch (std::system_error const& _error) { \
        SET_ERROR_CUSTOM(err \
            , _error.code().category().name() \
            , _error.code().value(), _error.what()); }

#define ERROR_CATCH_STD_EXCEPTION(err) \
    catch (std::exception const& _error) { \
        SET_ERROR_STRING(err, _error.what()); }

#define ERROR_CATCH_UNWIND(err) \
    catch (...) { SET_ERROR_STRING(err, "Unknown exception"); }

#if defined(YMH_ERR_WITH_BOOST_SYSTEM)
#   define ERROR_CATCH(err) \
        ERROR_CATCH_BOOST_SYSTEM(err) \
        ERROR_CATCH_STD_SYSTEM(err) \
        ERROR_CATCH_STD_EXCEPTION(err) \
        ERROR_CATCH_UNWIND(err)
#else
#   define ERROR_CATCH(err) \
        ERROR_CATCH_STD_SYSTEM(err) \
        ERROR_CATCH_STD_EXCEPTION(err) \
        ERROR_CATCH_UNWIND(err)
#endif  // YMH_ERR_WITH_BOOST_SYSTEM

//
// Wide String Supported.
//
#define __FILE__W ymh::err::detail::a2w(__FILE__)
#define __FUNCTION__W ymh::err::detail::a2w(__FUNCTION__)

#define SET_ERROR_CUSTOMW(errW, domain, val, fmt, ...) \
    do { errW.set_error_custom2(__FILE__W, __LINE__, __FUNCTION__W \
        , domain, val, fmt, ##__VA_ARGS__); } while (0)

#define SET_ERROR_MESSAGEW(errW, val, fmt, ...) \
    do { errW.set_error_message2(__FILE__W, __LINE__, __FUNCTION__W \
        , val, fmt, ##__VA_ARGS__); } while (0)

#define SET_ERROR_STRINGW(errW, fmt, ...) \
    do { errW.set_error_string2(__FILE__W, __LINE__, __FUNCTION__W \
        , fmt, ##__VA_ARGS__); } while (0)

#define SET_ERROR_CODEW(errW, ec) \
    do { errW.set_error_code2(__FILE__W, __LINE__, __FUNCTION__W, ec); } while (0)

#define MAKE_ERROR_CODEW(errW, e) \
    do { errW.make_error_code2(__FILE__W, __LINE__, __FUNCTION__W, e); } while (0)

#define SET_SYSTEM_ERRORW(errW, val) \
    do { errW.set_system_error2(__FILE__W, __LINE__, __FUNCTION__W, val); } while (0)

#define ERROR_TRYW try

#define ERROR_CATCH_BOOST_SYSTEMW(errW) \
    catch (boost::system::system_error const& _error) { \
        SET_ERROR_CUSTOMW(errW \
            , ymh::err::detail::a2w(_error.code().category().name()) \
            , _error.code().value(), ymh::err::detail::a2w(_error.what())); }

#define ERROR_CATCH_STD_SYSTEMW(errW) \
    catch (std::system_error const& _error) { \
        SET_ERROR_CUSTOMW(errW \
            , ymh::err::detail::a2w(_error.code().category().name()) \
            , _error.code().value(), ymh::err::detail::a2w(_error.what())); }

#define ERROR_CATCH_STD_EXCEPTIONW(errW) \
    catch (std::exception const& _error) { \
        SET_ERROR_STRINGW(errW, ymh::err::detail::a2w(_error.what())); } 

#define ERROR_CATCH_UNWINDW(errW) \
    catch (...) { SET_ERROR_STRINGW(errW, L"Unknown exception"); } 

#if defined(YMH_ERR_WITH_BOOST_SYSTEM)
#   define ERROR_CATCHW(errW) \
        ERROR_CATCH_BOOST_SYSTEMW(errW) \
        ERROR_CATCH_STD_SYSTEMW(errW) \
        ERROR_CATCH_STD_EXCEPTIONW(errW) \
        ERROR_CATCH_UNWINDW(errW)
#else
#   define ERROR_CATCHW(errW) \
        ERROR_CATCH_STD_SYSTEMW(errW) \
        ERROR_CATCH_STD_EXCEPTIONW(errW) \
        ERROR_CATCH_UNWINDW(errW)
#endif  // YMH_ERR_WITH_BOOST_SYSTEM


namespace ymh
{
namespace err
{
namespace detail
{
     
// 确保 l[w]printf 可变参数有效
//
// 如，可防止以下错误:
//      string_t s("hello");
//      std::printf("%s", s); // printf 不支持 string_t，但又不报错。
//
inline void ensure_va_args_safe_A() {}
template <class T, class... Args
          , typename std::enable_if<!std::is_class<T>::value
                                    && !std::is_same
                                    <typename std::remove_cv
                                     <typename std::remove_pointer
                                      <typename std::decay<T>::type
                                       >::type>::type, wchar_t>::value
                                    , int>::type = 0>
void ensure_va_args_safe_A(T const&, Args... args)
{
    ensure_va_args_safe_A(args...);
}

inline void ensure_va_args_safe_W() {}
template <class T, class... Args
          , typename std::enable_if<!std::is_class<T>::value
                                    && !std::is_same
                                    <typename std::remove_cv
                                     <typename std::remove_pointer
                                      <typename std::decay<T>::type
                                       >::type>::type, char>::value
                                    , int>::type = 0>
void ensure_va_args_safe_W(T const&, Args... args)
{
    ensure_va_args_safe_W(args...);
}

//
// s[w]printf 实现: 将可变参数格式化成字符串.
//
template <class charT>
struct sprintf_constructor;

template <>
struct sprintf_constructor<char>
{
    using char_type = char;
    using string_t = std::basic_string<char_type>;

    template <class... Args>
    static void construct(string_t& s, string_t const& fmt, Args&&... args)
    {
#if !defined(NDEBUG)
        ensure_va_args_safe_A(std::forward<Args>(args)...);
#endif
        for (int n = BUFSIZ; true; n += BUFSIZ)
        {
            s.resize(n--);
            auto r = std::snprintf(const_cast<char_type*>(s.data()), n
                                   , fmt.c_str(), args...);
            if (r < 0)
            {
                // TODO: throw system_error
                s.clear();
                break;
            }
            if (r < n)
            {
                s.resize(r);
                break;
            }
        }
    }
};

template <>
struct sprintf_constructor<wchar_t>
{
    using char_type = wchar_t;
    using string_t = std::basic_string<char_type>;

    template <class... Args>
    static void construct(string_t& s, string_t const& fmt, Args&&... args)
    {
#if !defined(NDEBUG)
        ensure_va_args_safe_W(std::forward<Args>(args)...);
#endif
        for (int n = BUFSIZ; true; n += BUFSIZ)
        {
            s.resize(n--);
            auto r = std::swprintf(const_cast<char_type*>(s.data()), n
                                   , fmt.c_str(), args...);
            if (r < 0)
            {
                // TODO: throw system_error
                s.clear();
                break;
            }
            if (r < n)
            {
                s.resize(r);
                break;
            }
        }
    }
};

template <class charT>
struct ansi_constructor;

template <>
struct ansi_constructor<char>
{
    using char_type = char;
    using string_t = std::basic_string<char_type>;

    template <class charFT
              , typename std::enable_if<std::is_same<charFT, char_type>::value
                                        , int>::type = 0>
    static void construct(string_t& to, std::basic_string<charFT> const& from)
    {
        to = from;
    }

    template <class charFT
              , typename std::enable_if<std::is_same<charFT, wchar_t>::value
                                        , int>::type = 0>
    static void construct(string_t& to, std::basic_string<charFT> const& from)
    {
        using from_type = charFT;
        using to_type = char_type;
        using cvt_facet = std::codecvt<from_type, to_type, std::mbstate_t>;

        constexpr std::size_t codecvt_buf_size = BUFSIZ;
        // perhaps too large, but that's OK
        // encodings like shift-JIS need some prefix space
        std::size_t buf_size = from.length() * 4 + 4;
        if (buf_size < codecvt_buf_size)
        {
            buf_size = codecvt_buf_size;
        }
        to.resize(buf_size);

        std::locale loc("");
        auto& cvt = std::use_facet<cvt_facet>(loc);
        do
        {
            from_type const* fb = from.data();
            from_type const* fe = from.data() + from.length();
            from_type const* fn = nullptr;

            to_type* tb = const_cast<to_type*>(to.data());
            to_type* te = const_cast<to_type*>(to.data() + to.length());
            to_type* tn = nullptr;

            std::mbstate_t state = std::mbstate_t();
            auto result = cvt.out(state, fb, fe, fn, tb, te, tn);
            if (result == cvt_facet::ok || result == cvt_facet::noconv)
            {
                to.resize(tn - tb);
                break;
            }
            else if (result == cvt_facet::partial)
            {
                buf_size += codecvt_buf_size;
                to.resize(buf_size);
                continue;
            }
            to.clear();
            break; // failed
        } while (true);
    }
};

template <>
struct ansi_constructor<wchar_t>
{
    using char_type = wchar_t;
    using string_t = std::basic_string<char_type>;

    template <class charFT
              , typename std::enable_if<std::is_same<charFT, char_type>::value
                                        , int>::type = 0>
    static void construct(string_t& to, std::basic_string<charFT> const& from)
    {
        to = from;
    }

    template <class charFT
              , typename std::enable_if<std::is_same<charFT, char>::value
                                        , int>::type = 0>
    static void construct(string_t& to, std::basic_string<charFT> const& from)
    {
        using from_type = charFT;
        using to_type = char_type;
        using cvt_facet = std::codecvt<to_type, from_type, std::mbstate_t>;

        constexpr std::size_t codecvt_buf_size = BUFSIZ;
        // perhaps too large, but that's OK
        std::size_t buf_size = from.length() * 3;
        if (buf_size < codecvt_buf_size)
        {
            buf_size = codecvt_buf_size;
        }
        to.resize(buf_size);

        std::locale loc("");
        auto& cvt = std::use_facet<cvt_facet>(loc);
        do
        {
            from_type const* fb = from.data();
            from_type const* fe = from.data() + from.length();
            from_type const* fn = nullptr;

            to_type* tb = const_cast<to_type*>(to.data());
            to_type* te = const_cast<to_type*>(to.data() + to.length());
            to_type* tn = nullptr;

            std::mbstate_t state = std::mbstate_t();
            auto result = cvt.in(state, fb, fe, fn, tb, te, tn);
            if (result == cvt_facet::ok || result == cvt_facet::noconv)
            {
                to.resize(tn - tb);
                break;
            }
            else if (result == cvt_facet::partial)
            {
                buf_size += codecvt_buf_size;
                to.resize(buf_size);
                continue;
            }
            to.clear();
            break; // failed
        } while (true);
    }
};

inline std::wstring a2w(std::string const& from)
{
    using char_type = std::wstring::value_type;
    std::basic_string<char_type> to;
    ansi_constructor<char_type>::construct(to, from);
    return to;
}

//
// Internal error value structure.
//
template <class charT>
class basic_errval
{
public:
    using char_type = charT;
    using string_t = std::basic_string<char_type>;

public:
    basic_errval(string_t const& file, int line, string_t const& func
        , string_t const& domain2, int val, string_t const& msg)
        : file_(file), line_(line), func_(func)
        , domain_(domain2), val_(val), msg_(msg) 
    {}
    
    basic_errval(string_t const& domain2, int val, string_t const& msg)
        : domain_(domain2), val_(val), msg_(msg) 
    {}
    
    basic_errval(string_t const& file, int line, string_t const& func
        , std::error_code const& ec)
        : file_(file), line_(line), func_(func)
    {
        std::string const cat_name(ec.category().name());
        ansi_constructor<char_type>::construct(domain_, cat_name);
        val_ = ec.value();
        ansi_constructor<char_type>::construct(msg_, ec.message());
    }
    
    explicit basic_errval(std::error_code const& ec)
    {
        std::string const cat_name(ec.category().name());
        ansi_constructor<char_type>::construct(domain_, cat_name);
        val_ = ec.value();
        ansi_constructor<char_type>::construct(msg_, ec.message());
    }

#if defined(YMH_ERR_WITH_BOOST_SYSTEM)

    basic_errval(string_t const& file, int line, string_t const& func
        , boost::system::error_code const& ec)
        : file_(file), line_(line), func_(func)
    {
        std::string const cat_name(ec.category().name());
        ansi_constructor<char_type>::construct(domain_, cat_name);
        val_ = ec.value();
        ansi_constructor<char_type>::construct(msg_, ec.message());
    }

    explicit basic_errval(boost::system::error_code const& ec)
    {
        std::string const cat_name(ec.category().name());
        ansi_constructor<char_type>::construct(domain_, cat_name);
        val_ = ec.value();
        ansi_constructor<char_type>::construct(msg_, ec.message());
    }

#endif  // YMH_ERR_WITH_BOOST_SYSTEM

    basic_errval() : basic_errval(std::error_code())
    {}
    
    string_t domain() const
    {
        return domain_;
    }

    int value() const
    {
        return val_;
    }

    string_t message() const
    {
        return msg_;
    }

public:
    void dump(std::basic_ostream<char_type>& ostrm, bool printfl=true) const
    {
        auto conv = [](std::string const& from) -> string_t
        {
            string_t to;
            ansi_constructor<char_type>::construct(to, from);
            return to;
        };
    
        if (printfl)
        {
            /*
            namespace fs = std::experimental::filesystem;

            fs::path file(file_);
            fs::path file_cp;
            auto const n = std::distance(file.begin(), file.end());
            auto it = file.begin();
            if (1 < n)
            {
                std::advance(it, n - 2);
                file_cp /= *it++;
            }
            if (0 < n)
            {
                file_cp /= *it++;
            }
            */
        
            string_t file_cp = file_;
            auto sep = ostrm.widen('/');
            auto r1st_sep = file_.find_last_of(sep);
            if (r1st_sep == string_t::npos)
            {
                sep = ostrm.widen('\\');
                r1st_sep = file_.find_last_of(sep);
            }
            if (r1st_sep != string_t::npos)
            {
                auto const r2nd_sep = file_.find_last_of(sep, r1st_sep - 1);
                if (r2nd_sep != string_t::npos)
                {
                    file_cp = file_.substr(r2nd_sep + 1);
                }
            }
            
            ostrm << conv("File \"") << file_cp 
                << conv("\", line ") << line_ << conv(", in ");
        }
        
        ostrm << func_ << conv(": ") << msg_;
        if (msg_.empty() || msg_.back() != '\n' || msg_.back() != L'\n')
        {
            ostrm << std::endl;
        }
    }

private:
    string_t file_;
    int line_ = 0;
    string_t func_;

private:
    string_t domain_;
    int val_ = 0;
    string_t msg_;
};

}  // namespace detail

//
// Error storage structure.
//
template <class charT>
class basic_error
{
public:
    using char_type = charT;
    using string_t = std::basic_string<char_type>;
    using errval_t = detail::basic_errval<char_type>;
    
public:
    template <class... Args>
    void set_error_custom(string_t const& domain2, int val
        , string_t const& fmt, Args&&... args)
    {
        errvals_.emplace_back(domain2
            , val, format(fmt, std::forward<Args>(args)...));
    }

    template <class... Args>
    void set_error_message(int val, string_t const& fmt, Args&&... args)
    {
        string_t const empty_str;
        set_error_custom(empty_str, val, fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    void set_error_string(string_t const& fmt, Args&&... args)
    {
        constexpr int val = (std::numeric_limits<int>::max)();
        set_error_message(val, fmt, std::forward<Args>(args)...);
    }

    void set_error_code(std::error_code const& ec)
    {
        errvals_.emplace_back(ec);
    }

    void make_error_code(std::errc e)
    {
        auto const ec = std::make_error_code(e);
        set_error_code(ec);
    }

#if defined(YMH_ERR_WITH_BOOST_SYSTEM)

    void set_error_code(boost::system::error_code const& ec)
    {
        errvals_.emplace_back(ec);
    }

    void make_error_code(boost::system::errc::errc_t e)
    {
        auto const ec = boost::system::errc::make_error_code(e);
        set_error_code(ec);
    }

#endif  // YMH_ERR_WITH_BOOST_SYSTEM

    void set_system_error(int val)
    {
        auto const ec = std::error_code(val, std::system_category());
        set_error_code(ec);
    }
    
 public:
    explicit operator bool() const
    {
        return !errvals_.empty();
    }

    string_t domain() const
    {
        return errvals_.empty() 
            ? std::error_code().category().name(): errvals_.front().domain();
    }

    int value() const
    {
        return errvals_.empty() 
            ? std::error_code().value() : errvals_.front().value();
    }

    string_t message() const
    {
        return errvals_.empty() 
            ? std::error_code().message() : errvals_.front().message();
    }

    void clear()
    {
        errvals_.clear();
    }

public:
    void dump(std::basic_ostream<char_type>& ostrm, bool printfl=true) const
    {
        if (errvals_.empty())
        {
            errval_t ev;
            ev.dump(ostrm, printfl);
        }
        errvals_.front().dump(ostrm, printfl);
    }
    
    string_t dump(bool printfl=true) const
    {
        std::basic_ostringstream<char_type> ostrm;
        dump(ostrm, printfl);
        return ostrm.str();
    }
    
    void dump_backtrace(std::basic_ostream<char_type>& ostrm, bool printfl=true) const
    {
        if (errvals_.empty())
        {
            dump(ostrm, printfl);
        }
        for (auto const& ev : errvals_)
        {
            ev.dump(ostrm, printfl);
        }
    }
    
    string_t dump_backtrace(bool printfl=true) const
    {
        std::basic_ostringstream<char_type> ostrm;
        dump_backtrace(ostrm, printfl);
        return ostrm.str();
    }
   
public:
    void dump_nofl(std::basic_ostream<char_type>& ostrm) const
    {
        dump(ostrm, false);
    }

    string_t dump_nofl() const
    {
        return dump(false);
    }

    void dump_backtrace_nofl(std::basic_ostream<char_type>& ostrm) const
    {
        dump_backtrace(ostrm, false);
    }

    string_t dump_backtrace_nofl() const
    {
        return dump_backtrace(false);
    }

public:
    template <class... Args>
    void set_error_custom2(string_t const& file, int line, string_t const& func
        , string_t const& domain2, int val, string_t const& fmt, Args&&... args)
    {
        errvals_.emplace_back(file, line, func
            , domain2, val, format(fmt, std::forward<Args>(args)...));
    }

    template <class... Args>
    void set_error_message2(string_t const& file, int line, string_t const& func
        , int val, string_t const& fmt, Args&&... args)
    {
        string_t const empty_str;
        set_error_custom2(file, line, func, empty_str
            , val, fmt, std::forward<Args>(args)...);
    }
    
    template <class... Args>
    void set_error_string2(string_t const& file, int line, string_t const& func
        , string_t const& fmt, Args&&... args)
    {
        constexpr int val = (std::numeric_limits<int>::max)();
        set_error_message2(file, line, func
            , val, fmt, std::forward<Args>(args)...);
    }
    
    void set_error_code2(string_t const& file, int line, string_t const& func
        , std::error_code const& ec)
    {
        errvals_.emplace_back(file, line, func, ec);
    }
    
    void make_error_code2(string_t const& file, int line, string_t const& func
        , std::errc e)
    {
        auto const ec = std::make_error_code(e);
        set_error_code2(file, line, func, ec);
    }
    
#if defined(YMH_ERR_WITH_BOOST_SYSTEM)

    void set_error_code2(string_t const& file, int line, string_t const& func
        , boost::system::error_code const& ec)
    {
        errvals_.emplace_back(file, line, func, ec);
    }

    void make_error_code2(string_t const& file, int line, string_t const& func
        , boost::system::errc::errc_t e)
    {
        auto const ec = boost::system::errc::make_error_code(e);
        set_error_code2(file, line, func, ec);
    }

#endif  // YMH_ERR_WITH_BOOST_SYSTEM

    void set_system_error2(string_t const& file, int line, string_t const& func
        , int val)
    {
        auto const ec = std::error_code(val, std::system_category());
        set_error_code2(file, line, func, ec);
    }
    
private:
    string_t format(string_t const& fmt) const
    {
        return fmt;
    }

    template <class... Args>
    string_t format(string_t const& fmt, Args&&... args) const
    {
        string_t s;
        detail::sprintf_constructor<char_type>::construct(s
            , fmt.c_str(), std::forward<Args>(args)...);
        return s;
    }
    
private:
    std::vector<errval_t> errvals_;
};

using error_t = basic_error<char>;
using werror_t = basic_error<wchar_t>;

}  // namespace err

using err::error_t;
using err::werror_t;

}  // namespace ymh

#endif  // YMH_ERROR_HPP

