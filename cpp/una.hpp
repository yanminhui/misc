/*
 * ======================
 * UNA: Unicode aNd Ansi
 * ====================== https://github.com/yanminhui/misc
 *
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 * Copyright (c) 2019 yanminhui <mailto:yanminhui163@163.com>.
 *
 * Unicode and ANSI (byte string) conversion is supported by class codec.
 *
 * Convert wide char to multi bytes by encode<codepage::cp_xxx, bom::nobomb> 
 * and reverse it by decode<codepage::cp_xxx, bom::nobomb>. It can convert one 
 * multi bytes to another multi bytes by convert<...>.
 *
 * Others, it will try convert to local codepage if you use file_text to read 
 * file's contents. You can save multi bytes to file by save_file_text<...>.
 *
 * Attention: may throw std::system_error exception if failed.
 *
 * [Function Prototype]
 *
 * (1) Multi Bytes to Wide Char
 *    
 *    std::wstring wtext = decode<codepage::cp_utf8>(u8"utf-8 string");
 *    wtext = UTF8ToUnicode<bom::nobomb>(u8"utf-8 string");"
 *
 * (2) Wide Char to Multi Bytes
 *
 *    std::string text = encode<codepage::cp_utf8>(L"wide string");
 *    text = UnicodeToUTF8<bom::bomb>(L"wide string");
 *
 * (3) Convert between Multi Bytes
 *   
 *    std::string ntext = convert<codepage::cp_utf8>("ansi string");
 *
 * (4) Read/Save File Text
 *
 *    std::wstring wtext = read_file_text(L"demo.txt");
 *    save_file_text<codepage::cp_utf8, bom::bomb>("demo.txt", "bingo");
 *
 * [Usage]
 *
 *    using namespace ymh;
 *
 *    try
 *    {
 *        auto text = file_text("demo.txt");
 *        auto wtext = decode(text);
 *    } 
 *    catch (std::system_error const& e)
 *    {
 *    }
 *   
 */
 
#ifndef YMH_UNA_HPP
#define YMH_UNA_HPP

#include <cmath>
#include <cstdint>

#include <algorithm>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

// For Windows
#if defined(_WIN32) || defined(_MSC_VER)
#   include <windows.h>
#endif  // defined(_WIN32) || defined(_MSC_VER)

// For GNU/Linux
#if defined(__GLIBC__) && __GLIBC__ >= 2 || defined(_ICONV_H)
#   if !defined(YMH_UNA_WITH_ICONV)
#       define YMH_UNA_WITH_ICONV 1
#   endif  // !defined(YMH_UNA_WITH_ICONV)
#endif  // defined(__GLIBC__) && __GLIBC__ >= 2 || defined(_ICONV_H)

// For MacOS
#if defined(__apple_build_version__) && __apple_build_version__ >= 10003000
#   if !defined(YMH_UNA_WITH_ICONV)
#       define YMH_UNA_WITH_ICONV 1
#   endif  // !defined(YMH_UNA_WITH_ICONV)
#endif  // defined(__apple_build_version__) && __apple_build_version__ >= 10003000

#if defined(_LIBICONV_H) || defined(_ICONV_H_)
#   if !defined(YMH_UNA_WITH_ICONV)
#       define YMH_UNA_WITH_ICONV 1
#   endif  // !defined(YMH_UNA_WITH_ICONV)
#endif  // defined(_LIBICONV_H) || defined(_ICONV_H_)

#if defined(YMH_UNA_WITH_ICONV)
#   include <iconv.h>
#endif  // YMH_UNA_WITH_ICONV

#if defined(_MSC_VER)
#   pragma warning(disable: 4018)
#endif

#define DEFAULT_CP ymh::una::codepage::cp_default
#define DEFAULT_BOM ymh::una::bom::nobomb

#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
#   define CONSTEXPR constexpr
#   define CP_DEFAULT_TEMPLATE_ARG = DEFAULT_CP
#   define BOM_DEFAULT_TEMPLATE_ARG = DEFAULT_BOM
#else
#   define CONSTEXPR const
#   define CP_DEFAULT_TEMPLATE_ARG
#   define BOM_DEFAULT_TEMPLATE_ARG
#endif

namespace ymh
{
namespace una  // Unicode aNd Ansi
{
namespace codepage
{
enum type {
    cp_default
    , cp_utf8
    , cp_gb2312
    , cp_gb18030
    , cp_ucs2_le
    , cp_ucs2_be
};
}  // namespace codepage::type

namespace bom  // Byte Order Mark
{
enum type {
    nobomb
    , bomb
};
}  // namespace bom

namespace detail
{

CONSTEXPR unsigned char g_utf8_bom[] = { 0xEF, 0xBB, 0xBF };
CONSTEXPR unsigned char g_gb18030_bom[] = { 0x84, 0x31, 0x95, 0x33 };
CONSTEXPR unsigned char g_ucs2_le_bom[] = { 0xFF, 0xFE };
CONSTEXPR unsigned char g_ucs2_be_bom[] = { 0xFE, 0xFF };

// <length, chars>
inline std::pair<int, unsigned char const*> get_bom(codepage::type cp)
{
    int size = 0;
    unsigned char const* chars = nullptr;

    using namespace codepage;
    switch (cp)
    {
    case cp_utf8:
        size = sizeof(g_utf8_bom) / sizeof(g_utf8_bom[0]);
        chars = g_utf8_bom;
        break;
    case cp_gb18030:
        size = sizeof(g_gb18030_bom) / sizeof(g_gb18030_bom[0]);
        chars = g_gb18030_bom;
        break;
    case cp_ucs2_le:
        size = sizeof(g_ucs2_le_bom) / sizeof(g_ucs2_le_bom[0]);
        chars = g_ucs2_le_bom;
        break;
    case cp_ucs2_be:
        size = sizeof(g_ucs2_be_bom) / sizeof(g_ucs2_be_bom[0]);
        chars = g_ucs2_be_bom;
        break;
    default:
        break;
    }
    return std::make_pair(size, chars);
}

template<class T>
struct default_del
{   
    // default deleter for unique_ptr
    typedef default_del<T> _Myt;

    default_del()
    {
    }

    template<class T2>
    default_del(default_del<T2> const&)
    {
    }

    void operator()(T *p) const
    {   // delete a pointer
        if (0 < sizeof (T) && p)    // won't compile for incomplete type
        {
            ::free(p);
        }
    }
};

typedef std::unique_ptr<char[], default_del<char> > char_ptr;
typedef std::unique_ptr<wchar_t[], default_del<wchar_t> > wchar_ptr;

}  // namespace detail

//============================================================================
// Abstract Platform Implement.
//
//============================================================================
namespace detail
{

template <class AllocatorEn, class AllocatorDe>
class codec_impl
{
public:
    codec_impl(codepage::type cp, bom::type bo) : cp_(cp), bom_(bo) 
    {}
    
    virtual ~codec_impl() 
    {}
    
    static std::shared_ptr<codec_impl> create_instance(codepage::type cp, bom::type bo);
    
    virtual void encode_impl(wchar_t const* wstr, int in_size
        , int& out_size, AllocatorEn allocator) const = 0;

    virtual void decode_impl(char const* bytes, int in_size
        , int& out_size, AllocatorDe allocator) const = 0;
    
protected:    
    std::pair<int, unsigned char const*> get_bom() const
    {
        if (bom_ == bom::nobomb)
        {
            int size = 0;
            unsigned char const* chars = nullptr;
            return std::make_pair(size, chars);
        }
        return detail::get_bom(this->cp_);
    }
    
protected:
    codepage::type cp_;
    bom::type bom_;
};

}  // namespace detail

//============================================================================
// Codec Bridge Interface.
//
//============================================================================
class codec
{
public:
    typedef detail::char_ptr char_ptr;
    typedef detail::wchar_ptr wchar_ptr;

    // Deprecated: Use ymh::bom::xx instead of ymh::codec::xx
    static CONSTEXPR auto bomb   = bom::bomb;
    static CONSTEXPR auto nobomb = bom::nobomb;

    // Deprecated: Use ymh::codepage:xx instead of ymh::codec::xx
    static CONSTEXPR auto cp_default = codepage::cp_default;
    static CONSTEXPR auto cp_utf8    = codepage::cp_utf8;
    static CONSTEXPR auto cp_gb2312  = codepage::cp_gb2312;
    static CONSTEXPR auto cp_gb18030 = codepage::cp_gb18030;
    static CONSTEXPR auto cp_ucs2_le = codepage::cp_ucs2_le;
    static CONSTEXPR auto cp_ucs2_be = codepage::cp_ucs2_be;

public:
    explicit codec(codepage::type cp = codepage::cp_default, bom::type bo = bom::nobomb)
        : cp_(cp), bom_(bo)
    {}
    
    virtual ~codec() 
    {}

    char_ptr encode(wchar_t const* wstr, int in_size, int& out_size) const
    {
        char_ptr out_ptr;
        int out_num = 0;
                
        encode_impl(wstr, in_size, out_size
            , [&out_ptr, &out_num](int n) -> char* 
            {
                out_num = n;
                char* cp = nullptr;
                
                if (out_ptr)
                {
                    cp = (char*)::realloc(out_ptr.release()
                        , out_num * sizeof(char));
                }
                else
                {
                    cp = (char*)::calloc(out_num, sizeof(char));
                }
                
                if (!cp)
                {
                    throw std::system_error(std::make_error_code(std::errc::not_enough_memory)
                        , "Allocate memory for encode");
                }

                char_ptr p(cp);
                out_ptr.swap(p);
                
                return out_ptr.get();
            });
            
        if (out_size < out_num)
        {
            out_ptr[out_size] = '\0';
        }
        return std::move(out_ptr);
    }
    
    std::string encode(std::wstring const& wstr) const
    {
        auto const in_size = wstr.size();
        if ((std::numeric_limits<int>::max)() < in_size)
        {
            throw std::system_error(std::make_error_code
                (std::errc::result_out_of_range)
                , "Input character's size is out of range");
        }

        int out_size = 0;
        std::string out;
        encode_impl(wstr.data(), static_cast<int>(in_size), out_size
            , [&out](int n) -> char*
            {
                out.resize(n);
                return const_cast<char*>(out.data());
            }
        );
        
        out.resize(out_size);
        return out;
    }

    wchar_ptr decode(char const* bytes, int in_size, int& out_size) const
    {
        wchar_ptr out_ptr;
        int out_num = 0;
        
        decode_impl(bytes, in_size, out_size
            , [&out_ptr, &out_num](int n) -> wchar_t*
            {
                out_num = n;
                wchar_t* cp = nullptr;
                
                if (out_ptr)
                {
                    cp = (wchar_t*)::realloc(out_ptr.release()
                        , out_num * sizeof(wchar_t));
                }
                else
                {
                    cp = (wchar_t*)::calloc(out_num, sizeof(wchar_t));
                }
                
                if (!cp)
                {
                    throw std::system_error(std::make_error_code(std::errc::not_enough_memory)
                        , "Allocate memory for decode");
                }

                wchar_ptr p(cp);
                out_ptr.swap(p);
                
                return out_ptr.get();
            }
        );
        
        if (out_size < out_num)
        {
            out_ptr.get()[out_size] = L'\0';
        }
        return std::move(out_ptr);        
    }
    
    std::wstring decode(std::string const& bytes) const
    {
        auto const in_size = bytes.size();
        if ((std::numeric_limits<int>::max)() < in_size)
        {
            throw std::system_error(std::make_error_code
                (std::errc::result_out_of_range)
                , "Input character's size is out of range");
        }

        std::wstring out;
        int out_size = 0;
        decode_impl(bytes.data(), static_cast<int>(in_size), out_size
            , [&out](int n) -> wchar_t*
            {
                out.resize(n);
                return const_cast<wchar_t*>(out.data());
            }
        );
        
        out.resize(out_size);
        return out;       
    }

public:
    char_ptr operator()(wchar_t const* wstr, int in_size, int& out_size) const
    {
        return encode(wstr, in_size, out_size);
    }
    
    std::string operator()(std::wstring const& wstr) const
    {
        return encode(wstr);
    }

    wchar_ptr operator()(char const* bytes, int in_size, int& out_size) const
    {
        return decode(bytes, in_size, out_size);
    }
    
    std::wstring operator()(std::string const& bytes) const
    {
        return decode(bytes);
    }
    
private:
    // allocator: char* allocator(int size);
    template <class AllocatorT>
    void encode_impl(wchar_t const* wstr, int in_size
        , int& out_size, AllocatorT allocator) const
    {
       auto ci = detail::codec_impl<AllocatorT, wchar_t*(int)>::create_instance(cp_, bom_);
       return ci->encode_impl(wstr, in_size, out_size, allocator);
    }

    // allocator: wchar_t* allocator(int size);
    template <class AllocatorT>
    void decode_impl(char const* bytes, int in_size
        , int& out_size, AllocatorT allocator) const
    {
        auto ci = detail::codec_impl<char*(int), AllocatorT>::create_instance(cp_, bom_);
        return ci->decode_impl(bytes, in_size, out_size, allocator);
    }
    
private:
    codepage::type cp_;
    bom::type bom_;
};  

//============================================================================
// Windows Implement.
//
//============================================================================
#if defined(_WIN32) || defined(_MSC_VER)

//
// WideCharToMultiByte、MultiByteToWideChar Implement.
//
namespace detail
{
namespace ucs2_le
{

inline int WideCharToMultiByte(wchar_t const* wstr, int in_size
    , char* out, int out_size)
{
    auto const in_byte_size = in_size * static_cast<int>
        (sizeof(wchar_t) / sizeof(char));
    if (out_size == 0)
    {
        return in_byte_size;
    }

    auto in = (unsigned char const*)wstr;
    for (int i = 0
        ; i != (in_byte_size < out_size ? in_byte_size : out_size)
        ; ++i)
    {
        *(out + i) = *(in + i);
    }
    return out_size;
}

inline int MultiByteToWideChar(char const* bytes, int in_size
    , wchar_t* out, int out_size)
{
    auto const out_wchar_size = in_size / static_cast<int>
        (sizeof(wchar_t) / sizeof(char));
    if (out_size == 0)
    {
        return out_wchar_size;
    }

    auto p_out = (unsigned char*)out;
    auto const out_byte_size = out_size * static_cast<int>
        (sizeof(wchar_t) / sizeof(char));
    for (int i = 0
        ; i != (in_size < out_byte_size ? in_size : out_byte_size)
        ; ++i)
    {
        *(p_out + i) = *(bytes + i);
    }
    return out_size;
}

inline bool is_ucs2_le(char const* bytes, int in_size)
{
    if (in_size % 2)
    // ucs2 byte size is even
    {
        --in_size;
    }

    return ::IsTextUnicode(reinterpret_cast<void const*>(bytes)
        , in_size, nullptr) ? true : false;
}

inline bool is_ucs2_le(std::string const& bytes)
{
    auto const in_size = bytes.size();
    if ((std::numeric_limits<int>::max)() < in_size)
    {
        throw std::system_error(std::make_error_code
            (std::errc::result_out_of_range)
            , "Input character's size is out of range");
    }

    return is_ucs2_le(const_cast<char const*>(bytes.data())
        , static_cast<int>(in_size));
}

} // namespace ucs2_le

namespace ucs2_be
{

inline int WideCharToMultiByte(wchar_t const* wstr, int in_size
    , char* out, int out_size)
{
    auto const in_byte_size = in_size * static_cast<int>
        (sizeof(wchar_t) / sizeof(char));
    if (out_size == 0)
    {
        return in_byte_size;
    }

    auto in = (unsigned char const*)wstr;
    for (int i = 0
        ; i != (in_byte_size < out_size ? in_byte_size : out_size)
        ; ++i)
    {
        *(out + i) = *(in + ((i % 2 == 0) ? (i + 1) : (i - 1)));
    }
    return out_size;
}

inline int MultiByteToWideChar(char const* bytes, int in_size
    , wchar_t* out, int out_size)
{
    auto const out_wchar_size = in_size / static_cast<int>
        (sizeof(wchar_t) / sizeof(char));
    if (out_size == 0)
    {
        return out_wchar_size;
    }

    auto p_out = (unsigned char*)out;
    auto const out_byte_size = out_size * static_cast<int>
        (sizeof(wchar_t) / sizeof(char));
    for (int i = 0
        ; i != (in_size < out_byte_size ? in_size : out_byte_size)
        ; ++i)
    {
        *(p_out + i) = *(bytes + ((i % 2 == 0) ? (i + 1) : (i - 1)));
    }
    return out_size;
}

} // namespace ucs2_be

inline int to_win_codepage(codepage::type cp)
{
    using namespace codepage;
    switch(cp)
    {
    case cp_default: return CP_ACP;
    case cp_utf8:    return CP_UTF8;
    case cp_gb2312:  return 936;
    case cp_gb18030: return 54936;
    case cp_ucs2_le: return 1200;
    case cp_ucs2_be: return 1201;
    }
    return cp;
}

inline int WideCharToMultiByte(codepage::type cp
    , wchar_t const* wstr, int in_size
    , char* out, int out_size)
{
    using namespace codepage;
    switch (cp)
    {
    case cp_ucs2_le:
        return ucs2_le::WideCharToMultiByte(wstr, in_size, out, out_size);
    case cp_ucs2_be:
        return ucs2_be::WideCharToMultiByte(wstr, in_size, out, out_size);
    default:
        break;
    }
    return ::WideCharToMultiByte(to_win_codepage(cp), 0
        , wstr, in_size
        , out, out_size
        , nullptr, nullptr);
}

inline int MultiByteToWideChar(codepage::type cp
    , char const* bytes, int in_size
    , wchar_t* out, int out_size)
{
    using namespace codepage;
    switch (cp)
    {
    case cp_ucs2_le:
        return ucs2_le::MultiByteToWideChar(bytes, in_size, out, out_size);
    case cp_ucs2_be:
        return ucs2_be::MultiByteToWideChar(bytes, in_size, out, out_size);
    default:
        break;
    }
    return ::MultiByteToWideChar(to_win_codepage(cp), 0
        , bytes, in_size
        , out, out_size);
}

//
// Windows Codec Implement.
//
template <class AllocatorEn, class AllocatorDe>
class win_impl : public codec_impl<AllocatorEn, AllocatorDe>
{
public:
    win_impl(codepage::type cp, bom::type bo)
        : codec_impl<AllocatorEn, AllocatorDe>(cp, bo)
    {}
    
    virtual void encode_impl(wchar_t const* wstr, int in_size
        , int& out_size, AllocatorEn allocator) const
    {
        auto const bom_size = this->get_bom().first;
        auto const bom_chars = this->get_bom().second;

        char* out = nullptr;
        for (out_size = 0; !out; out_size += bom_size)
        {
            if (!in_size)
            // fill BOM if in_size == 0, then exit.
            {
                out = allocator(bom_size);
                if (bom_chars)
                {
                    // std::copy(bom_chars, bom_chars + bom_size, out);
                    // out += bom_size;
                    for (auto i = 0; i != bom_size; ++i)
                    {
                        *out++ = *(bom_chars + i);
                    }
                }
                return ;
            }
            else if (out_size != 0)
            {
                out = allocator(out_size);
                if (bom_chars)
                {
                    // std::copy(bom_chars, bom_chars + bom_size, out);
                    // out += bom_size;
                    for (auto i = 0; i != bom_size; ++i)
                    {
                        *out++ = *(bom_chars + i);
                    }
                }
            }
            out_size = detail::WideCharToMultiByte(this->cp_
                , wstr, in_size
                , out, out_size);
            if (out_size == 0)
            {
                throw std::system_error(std::error_code(::GetLastError()
                    , std::system_category())
                    , "Failed encode");
            }
        }    
    }
    
    virtual void decode_impl(char const* bytes, int in_size
        , int& out_size, AllocatorDe allocator) const
    {
        auto const bom_size = this->get_bom().first;
        auto const bom_chars = this->get_bom().second;
        if (bom_chars && in_size >= bom_size)
        // skip bom chars
        {
            auto p_bytes = bytes;
            for (auto p_bom = bom_chars 
                ; p_bom < bom_chars + bom_size
                ; ++p_bom, ++p_bytes)
            {
                if (*p_bom != static_cast<decltype(*p_bom)>(*p_bytes))
                {
                    break;
                }
            }
            if (p_bytes - bytes == bom_size)
            {
                bytes   += bom_size;
                in_size -= bom_size;
            }
        }

        wchar_t* out = nullptr;
        for (out_size = 0; !out && in_size; )
        // exit if in_size == 0
        {
            if (out_size != 0)
            {
                out = allocator(out_size);
            }
            out_size = detail::MultiByteToWideChar(this->cp_
                , bytes, in_size
                , out, out_size);
            if (out_size == 0)
            {
                throw std::system_error(std::error_code(::GetLastError()
                    , std::system_category())
                    , "Failed decode");
            }
        }
    }
};

} // namespace detail

#endif  // defined(_WIN32) || defined(_MSC_VER)
//============================================================================
// IConv Implement.
//
//============================================================================
#if defined(YMH_UNA_WITH_ICONV)

namespace detail
{

inline std::string default_wide_charset()
{
    union {
        std::uint16_t n;
        unsigned char c[sizeof(std::uint16_t)];
    };
    n = 0x0102;
    
    auto const big_endian = (c[0] == 0x01 && c[1] == 0x02);
    auto const byte_size = sizeof(wchar_t);

    if (byte_size == 4)
    {
        return big_endian ? "UCS-4BE" : "UCS-4LE";
    }
    return big_endian ? "UCS-2BE" : "UCS-2LE";
}

inline std::string to_iconv_codepage(codepage::type cp)
{
    using namespace codepage;
    switch(cp)
    {
    case cp_default: 
    {
        std::locale loc("");
        auto name = loc.name();
        auto bp = name.find('.');
        auto ep = name.find('@', bp);
        return name.substr(bp == std::string::npos ? bp : bp + 1, ep);    
    }
    case cp_utf8:    return "UTF-8";
    case cp_gb2312:  return "CP936";
    case cp_gb18030: return "GB18030";
    case cp_ucs2_le: return "UCS-2LE";
    case cp_ucs2_be: return "UCS-2BE";
    }
    return "";
}

template <class AllocatorEn, class AllocatorDe>
class iconv_impl : public codec_impl<AllocatorEn, AllocatorDe>
{
public:
    iconv_impl(codepage::type cp, bom::type bo)
        : codec_impl<AllocatorEn, AllocatorDe>(cp, bo)
    {}

    virtual void encode_impl(wchar_t const* wstr, int in_size
        , int& out_size, AllocatorEn allocator) const
    {
        auto const bom_size = this->get_bom().first;
        auto const bom_chars = this->get_bom().second;

        char* out = nullptr;
        for (out_size = 0; !out; out_size += bom_size)
        {
            if (!in_size)
            // fill BOM if in_size == 0, then exit.
            {
                out = allocator(bom_size);
                if (bom_chars)
                {
                    // std::copy(bom_chars, bom_chars + bom_size, out);
                    // out += bom_size;
                    for (auto i = 0; i != bom_size; ++i)
                    {
                        *out++ = *(bom_chars + i);
                    }
                }
                return ;
            }
            else if (out_size != 0)
            {
                out = allocator(out_size);
                if (bom_chars)
                {
                    // std::copy(bom_chars, bom_chars + bom_size, out);
                    // out += bom_size;
                    for (auto i = 0; i != bom_size; ++i)
                    {
                        *out++ = *(bom_chars + i);
                    }
                }
            }

            if (out_size == 0)
            // calculate out buffer size.
            {
                // perhaps too large, but that's OK
                // encodings like shift-JIS need some prefix space
                CONSTEXPR auto ratio = 4;
                out_size = in_size * ratio + 4;
                continue ;
            }

            if (this->cp_ == codepage::cp_default)
            // Unicode to ANSI
            {
                typedef wchar_t from_type;
                typedef char to_type;
                typedef std::codecvt<from_type, to_type, std::mbstate_t> cvt_facet;

                std::locale loc("");
                auto& cvt = std::use_facet<cvt_facet>(loc);
                for ( ; ; )
                {
                    from_type const* fb = wstr;
                    from_type const* fe = wstr + in_size;
                    from_type const* fn = nullptr;

                    to_type* tb = const_cast<to_type*>(out);
                    to_type* te = const_cast<to_type*>(out + out_size - bom_size);
                    to_type* tn = nullptr;

                    std::mbstate_t state = std::mbstate_t();
                    auto result = cvt.out(state, fb, fe, fn, tb, te, tn);
                    if (result == cvt_facet::ok || result == cvt_facet::noconv)
                    {
                        out_size = (tn - tb) + bom_size;
                        return ;
                    }
                    else if (result == cvt_facet::partial)
                    {
                        out_size += BUFSIZ;
                        out = allocator(out_size) + bom_size;
                        continue;
                    }
                    
                    // failed
                    throw std::system_error(std::make_error_code(std::errc::illegal_byte_sequence)
                        , "std::codecvt for encode");
                }
                return ;
            }

            // open
            auto cd = ::iconv_open(to_iconv_codepage(this->cp_).c_str(), default_wide_charset().c_str());
            if (cd == (iconv_t)(-1))
            {
                throw std::system_error(std::error_code(errno
                    , std::system_category())
                    , "iconv_open for encode");
            }

            // iconv
            char* inbuf = (char*)wstr;
            size_t inbytesleft = sizeof(wchar_t) * in_size;
            char* outbuf = out;
            size_t outbytesleft = out_size - bom_size;                  
            
            ::iconv(cd, nullptr, nullptr, nullptr, nullptr);
            for ( ; ; )
            {
                auto rv = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
                if (rv == static_cast<size_t>(-1))
                {
                    if (errno == E2BIG)
                    // outbuf is too small, increment its size.
                    {
                        outbuf = allocator(out_size + BUFSIZ) + (out_size - outbytesleft);
                        outbytesleft += BUFSIZ;
                        out_size += BUFSIZ;
                        continue;
                    }

                    ::iconv_close(cd);
                    throw std::system_error(std::error_code(errno
                        , std::system_category())
                        , "iconv for encode");
                }
                
                out_size -= outbytesleft;
                break;
            }
            
            // close
            if (::iconv_close(cd) == static_cast<size_t>(-1))
            {
                throw std::system_error(std::error_code(errno
                    , std::system_category())
                    , "iconv_close for encode");
            }
        }    
    }

    virtual void decode_impl(char const* bytes, int in_size
        , int& out_size, AllocatorDe allocator) const
    {
        auto const bom_size = this->get_bom().first;
        auto const bom_chars = this->get_bom().second;
        if (bom_chars && in_size >= bom_size)
        // skip bom chars
        {
            auto p_bytes = bytes;
            for (auto p_bom = bom_chars 
                ; p_bom < bom_chars + bom_size
                ; ++p_bom, ++p_bytes)
            {
                if (*p_bom != static_cast<decltype(*p_bom)>(*p_bytes))
                {
                    break;
                }
            }
            if (p_bytes - bytes == bom_size)
            {
                bytes   += bom_size;
                in_size -= bom_size;
            }
        }

        wchar_t* out = nullptr;
        for (out_size = 0; !out && in_size; )
        // exit if in_size == 0
        {
            if (out_size != 0)
            {
                out = allocator(out_size);
            }
            else if (out_size == 0)
            // calculate out buffer size.
            {
                // perhaps too large, but that's OK
                CONSTEXPR auto ratio = 3;
                out_size = in_size * ratio;
                continue ;
            }
            
            if (this->cp_ == codepage::cp_default)
            // ANSI to Unicode
            {
                typedef char from_type;
                typedef wchar_t to_type;
                typedef std::codecvt<to_type, from_type, std::mbstate_t> cvt_facet;

                std::locale loc("");
                auto& cvt = std::use_facet<cvt_facet>(loc);
                for ( ; ; )
                {
                    from_type const* fb = bytes;
                    from_type const* fe = bytes + in_size;
                    from_type const* fn = nullptr;

                    to_type* tb = const_cast<to_type*>(out);
                    to_type* te = const_cast<to_type*>(out + out_size);
                    to_type* tn = nullptr;

                    std::mbstate_t state = std::mbstate_t();
                    auto result = cvt.in(state, fb, fe, fn, tb, te, tn);
                    if (result == cvt_facet::ok || result == cvt_facet::noconv)
                    {
                        out_size = tn - tb;
                        return ;
                    }
                    else if (result == cvt_facet::partial)
                    {
                        out_size += BUFSIZ;
                        out = allocator(out_size);
                        continue;
                    }

                    // failed
                    throw std::system_error(std::make_error_code(std::errc::illegal_byte_sequence)
                        , "std::codecvt for encode");
                }
                return ;
            }

            // open
            auto cd = ::iconv_open(default_wide_charset().c_str(), to_iconv_codepage(this->cp_).c_str());
            if (cd == (iconv_t)(-1))
            {
                throw std::system_error(std::error_code(errno
                    , std::system_category())
                    , "iconv_open for decode");
            }
            
            // iconv
            char* inbuf = (char*)bytes;
            size_t inbytesleft = in_size;
            char* outbuf = (char*)out;
            size_t outbytesleft = out_size * sizeof(wchar_t);               
            
            ::iconv(cd, nullptr, nullptr, nullptr, nullptr);
            for ( ; ; )
            {
                auto rv = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
                if (rv == static_cast<size_t>(-1))
                {
                    if (errno == E2BIG)
                    // outbuf is too small, increment its size.
                    {
                        outbuf = (char*)allocator(out_size + BUFSIZ) + (out_size * sizeof(wchar_t) - outbytesleft);
                        outbytesleft += BUFSIZ * sizeof(wchar_t);
                        out_size += BUFSIZ;
                        continue;
                    }
                    
                    ::iconv_close(cd);
                    throw std::system_error(std::error_code(errno
                        , std::system_category())
                        , "iconv for decode");
                }

                out_size -= outbytesleft / sizeof(wchar_t);
                break;
            }

            // close
            if (::iconv_close(cd) == static_cast<size_t>(-1))
            {
                throw std::system_error(std::error_code(errno
                    , std::system_category())
                    , "iconv_close for decode");
            }
        }
    }
};

}  // namespace detail

#endif  // YMH_UNA_WITH_ICONV
//============================================================================
// Create Codec Implement Instance.
//
//============================================================================
namespace detail
{
 
template <class AllocatorEn, class AllocatorDe>
std::shared_ptr<codec_impl<AllocatorEn, AllocatorDe>> 
codec_impl<AllocatorEn, AllocatorDe>::create_instance(codepage::type cp, bom::type bo)
{
#if defined(_WIN32) || defined(_MSC_VER)
    typedef win_impl<AllocatorEn, AllocatorDe> codec_type;
#elif defined(YMH_UNA_WITH_ICONV)
    typedef iconv_impl<AllocatorEn, AllocatorDe> codec_type;
#else
#   error requires include <iconv.h> before una.hpp
#endif // defined(_WIN32) || defined(_MSC_VER)
    return std::make_shared<codec_type>(cp, bo);
}

}  // namespace detail

//============================================================================
// Convenient User Interface. 
//
//============================================================================

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
        , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string encode(std::wstring const& wstr)
{
    return codec(cp, bo)(wstr);
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
        , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::char_ptr encode(wchar_t const* wstr, int in_size, int& out_size)
{
    return std::move(codec(cp, bo)(wstr, in_size, out_size));
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::wstring decode(std::string const& bytes)
{
    return codec(cp, bo)(bytes);
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::wchar_ptr decode(char const* bytes, int in_size, int& out_size)
{
    return std::move(codec(cp, bo)(bytes, in_size, out_size));
}

template <codepage::type from_cp CP_DEFAULT_TEMPLATE_ARG
    , codepage::type to_cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type from_bo BOM_DEFAULT_TEMPLATE_ARG
    , bom::type to_bo BOM_DEFAULT_TEMPLATE_ARG
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
    , typename std::enable_if<from_cp != to_cp || from_bo != to_bo, int>::type = 0 >
#else
    >
#endif
inline codec::char_ptr convert(char const* bytes, int in_size, int& out_size)
{
    auto p = decode<from_cp, from_bo>(bytes, in_size, out_size);
    return encode<to_cp, to_bo>(p.get(), out_size, out_size);
}

#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
template <codepage::type from_cp CP_DEFAULT_TEMPLATE_ARG
    , codepage::type to_cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type from_bo BOM_DEFAULT_TEMPLATE_ARG
    , bom::type to_bo BOM_DEFAULT_TEMPLATE_ARG
    , typename std::enable_if<from_cp == to_cp && from_bo == to_bo, int>::type = 0>
inline codec::char_ptr convert(char const* bytes, int in_size, int& out_size)
{
    // TODO: have better ?
    codec::char_ptr to(new char[in_size]);
    std::copy(bytes, bytes + in_size, to.get());
    out_size = in_size;
    return std::move(to);
}
#else
    // need c++11
#endif

template <codepage::type from_cp CP_DEFAULT_TEMPLATE_ARG
    , codepage::type to_cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type from_bo BOM_DEFAULT_TEMPLATE_ARG
    , bom::type to_bo BOM_DEFAULT_TEMPLATE_ARG
#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
    , typename std::enable_if<from_cp != to_cp || from_bo != to_bo, int>::type = 0>
#else
    >
#endif
inline std::string convert(std::string const& bytes)
{
    return encode<to_cp, to_bo>(decode<from_cp, from_bo>(bytes));
}

#if __cplusplus >= 201103L || (defined(_MSC_VER) && _MSC_VER >= 1900)
template <codepage::type from_cp CP_DEFAULT_TEMPLATE_ARG
    , codepage::type to_cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type from_bo BOM_DEFAULT_TEMPLATE_ARG
    , bom::type to_bo BOM_DEFAULT_TEMPLATE_ARG
    , typename std::enable_if<from_cp == to_cp && from_bo == to_bo, int>::type = 0>
inline std::string convert(std::string const& bytes)
{
    return bytes;
}
#else
    // need c++11
#endif

// UnicodeToANSI
template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string UnicodeToANSI(std::wstring const& wstr)
{
    return encode<codepage::cp_default, bo>(wstr);
}

template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::char_ptr UnicodeToANSI(wchar_t const* wstr, int in_size, int& out_size)
{
    return std::move(encode<codepage::cp_default, bo>(wstr, in_size, out_size));
}

// ANSIToUnicode
template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::wstring ANSIToUnicode(std::string const& bytes)
{
    return decode<codepage::cp_default, bo>(bytes);
}

template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::wchar_ptr ANSIToUnicode(char const* bytes, int in_size, int& out_size)
{
    return std::move(decode<codepage::cp_default, bo>(bytes, in_size, out_size));
}

// UnicodeToUTF8
template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string UnicodeToUTF8(std::wstring const& wstr)
{
    return encode<codepage::cp_utf8, bo>(wstr);
}

template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::char_ptr UnicodeToUTF8(wchar_t const* wstr, int in_size, int& out_size)
{
    return std::move(encode<codepage::cp_utf8, bo>(wstr, in_size, out_size));
}

// UTF8ToUnicode
template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::wstring UTF8ToUnicode(std::string const& bytes)
{
    return decode<codepage::cp_utf8, bo>(bytes);
}

template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::wchar_ptr UTF8ToUnicode(char const* bytes, int in_size, int& out_size)
{
    return std::move(decode<codepage::cp_utf8, bo>(bytes, in_size, out_size));
}

// UnicodeToGB2312
template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string UnicodeToGB2312(std::wstring const& wstr)
{
    return encode<codepage::cp_gb2312, bo>(wstr);
}

template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::char_ptr UnicodeToGB2312(wchar_t const* wstr, int in_size, int& out_size)
{
    return std::move(encode<codepage::cp_gb2312, bo>(wstr, in_size, out_size));
}

// GB2312ToUnicode
template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::wstring GB2312ToUnicode(std::string const& bytes)
{
    return decode<codepage::cp_gb2312, bo>(bytes);
}

template <bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline codec::wchar_ptr GB2312ToUnicode(char const* bytes, int in_size, int& out_size)
{
    return std::move(decode<codepage::cp_gb2312, bo>(bytes, in_size, out_size));
}

}  // namespace una

namespace codepage = una::codepage;
namespace bom = una::bom;

using una::codec;
using una::encode;
using una::decode;
using una::convert;

using una::UnicodeToANSI;
using una::ANSIToUnicode;

using una::UnicodeToUTF8;
using una::UTF8ToUnicode;

using una::UnicodeToGB2312;
using una::GB2312ToUnicode;

}  // namespace ymh

//===========================================================================// 
// Others
//
//===========================================================================//
namespace ymh
{
namespace una
{
namespace detail
{
namespace utf8
{

// byte is utf8 character?
//
// requires：
//     - bytes length >= step;
//     - 1 <= step <= 6.
//
// @note ascii if step == 1
//
// return step if true else 0.
inline std::uint8_t step_bytes(char const* bytes, int /*in_size*/, std::uint8_t step)
{
    // single byte (ascii)
    //    0xxxxxxx
    // -> 00000000
    if (step == 1)
    {
        return ((static_cast<unsigned char>(*bytes) >> 7) == 0) ? 1 : 0;
    }

    // invalid
    if (step > 6)
    {
        return 0;
    }

    // multi bytes
    //      110xxxxx 10xxxxxx
    //  ->  00000110 00000010
    // 
    // step  = 2
    // 5     = 7 - step         = 8 - step [x 1] - 1 [x 0]
    // 0x06  = 2^(step + 1) - 2 = 2^(2 + 1) - 2
    // 
    //      1110xxxxx 10xxxxxx 10xxxxxx
    //  ->  000001110 00000010 00000010  
    //
    // step  = 3
    // 4     = 7 - step         = 8 - step [x 1] - 1 [x 0] = 5
    // 0x0E  = 2^(step + 1) - 2 = 2^(3 + 1) - 2
    if ((static_cast<unsigned char>(*bytes++) >> (7 - step)) 
        != static_cast<std::uint8_t>(std::pow(2.0, step + 1) - 2))
    {
        return 0;
    }

    for (std::uint8_t i = 0; i != (step - 1); ++i)
    {
        if (static_cast<unsigned char>(*bytes++) >> 6 != 0x02)
        {
            return 0;
        }
    }

    return step;
}

inline bool is_utf8(char const* bytes, int in_size)
{   
    for (int i = 0; i != in_size; )
    {
        CONSTEXPR std::uint8_t utf8_max_bytes = 6;
        std::uint8_t const k = (in_size - i > utf8_max_bytes)
            ? utf8_max_bytes : static_cast<std::uint8_t>(in_size - i);

        for (std::uint8_t j = 1; j <= k; ++j)
        // [1, utf8_max_bytes]
        {
            if (step_bytes(bytes + i, in_size, j))
            {
                // @warning quick check:
                //          comment if (j >= 2) scoped code。
                // if (j >= 2)
                // {
                //   return true;
                // }

                i += j;
                break;
            }
            else if (j == k)
            {
                // @warning relax check
                return (k < utf8_max_bytes);
                // return false;
            }
        }
    }
    return true;
}

inline bool is_utf8(std::string const& bytes)
{
    auto const in_size = bytes.size();
    if ((std::numeric_limits<int>::max)() < in_size)
    {
        throw std::system_error(std::make_error_code
            (std::errc::result_out_of_range)
            , "Input character's size is out of range");
    }
    
    return is_utf8(const_cast<char const*>(bytes.data())
        , static_cast<int>(in_size));
}

}  // namespace utf8

inline codepage::type hint_codepage(char const* bytes, int in_size, bom::type* bo = nullptr)
{
    // check bom bytes order mark.
    using namespace codepage;
    static codepage::type const cps[] = { cp_default
                            , cp_utf8
                            , cp_gb2312
                            , cp_gb18030
                            , cp_ucs2_le
                            , cp_ucs2_be };
    size_t const cps_size = sizeof(cps) / sizeof(*cps);
    
    for (size_t i = 0; i != cps_size; ++i)
    {
        auto const bom = get_bom(cps[i]);
        auto const bom_size = bom.first;
        auto const bom_chars = bom.second;
        
        if (!bom_chars || in_size < bom_size)
        {
            continue;
        }
        
        auto p_bytes = bytes;
        for (auto p_bom = bom_chars; p_bom < bom_chars + bom_size
            ; ++p_bom, ++p_bytes)
        {
            if (*p_bom != static_cast<decltype(*p_bom)>(*p_bytes))
            {
                break;
            }
        }
        
        if (p_bytes - bytes == bom_size)
        {
            if (bo)
            {
                *bo = bom::bomb;
            }
            return cps[i];
        }
    }
    
    // check content.
 #if defined(_WIN32)
    if (ucs2_le::is_ucs2_le(bytes, in_size))
    {
        if (bo)
        {
            *bo = bom::nobomb;
        }
        return cp_ucs2_le;
    }
#endif  // _WIN32

    if (utf8::is_utf8(bytes, in_size))
    {
        if (bo)
        {
            *bo = bom::nobomb;
        }
        return cp_utf8;
    }
    
    return cp_default; // can't check codepage
}

inline codepage::type hint_codepage(std::string const& bytes, bom::type* bo = nullptr)
{
    auto const in_size = bytes.size();
    if ((std::numeric_limits<int>::max)() < in_size)
    {
        throw std::system_error(std::make_error_code
            (std::errc::result_out_of_range)
            , "Input character's size is out of range");
    }
    
    return hint_codepage(const_cast<char const*>(bytes.data())
        , static_cast<int>(in_size), bo);
}

}  // namespace detail

inline codepage::type hint_codepage(char const* bytes, int in_size, bom::type& bo)
{
    return detail::hint_codepage(bytes, in_size, &bo);
}

inline codepage::type hint_codepage(char const* bytes, int in_size)
{
    return detail::hint_codepage(bytes, in_size);
}

inline codepage::type hint_codepage(std::string const& bytes, bom::type& bo)
{
    return detail::hint_codepage(bytes, &bo);
}

inline codepage::type hint_codepage(std::string const& bytes)
{
    return detail::hint_codepage(bytes);
}

inline bool is_ascii(char const* bytes, int in_size)
{
    for (int i = 0; i != in_size; ++i)
    {
        if (static_cast<unsigned char>(*bytes++) >> 7)
        {
            return false;
        }
    }
    return true;
}

inline bool is_ascii(std::string const& bytes)
{
    auto const in_size = bytes.size();
    if ((std::numeric_limits<int>::max)() < in_size)
    {
        throw std::system_error(std::make_error_code
            (std::errc::result_out_of_range)
            , "Input character's size is out of range");
    }
    
    return is_ascii(const_cast<char const*>(bytes.data())
        , static_cast<int>(in_size));
}

template <class CharT>
inline std::string file_data(CharT const* filename)
{
    std::string text;
    
    std::ifstream ifile;
    ifile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try
    {
        ifile.open(filename, std::ifstream::in | std::ifstream::binary);
        std::filebuf* fbuf = ifile.rdbuf();
        do
        {
            auto const fbuf_size = fbuf->in_avail();
            if (fbuf_size < 0)
            {
                break;
            }
            auto const text_size = text.size();
            text.resize(text_size + static_cast<std::string::size_type>(fbuf_size));

            auto const sget_size = fbuf->sgetn((char*)(text.data() + text_size)
                , fbuf_size);
            if (sget_size != fbuf_size)
            {
                text.erase(text_size + static_cast<std::string::size_type>(sget_size));
            }
        } while (fbuf->sgetc() != std::ifstream::traits_type::eof());
    }
    catch (std::ifstream::failure const& e)
    {
        throw std::system_error(std::make_error_code
            (std::errc::no_such_file_or_directory), e.what());
    }
    
    return std::move(text);
}

#if !defined(_MSC_VER)
template <>
inline std::string file_data<wchar_t>(wchar_t const* wfilename)
{
    return file_data<char>(codec(codepage::cp_default
        , bom::nobomb).encode(wfilename).c_str());
}
#endif  // !_MSC_VER

inline std::string file_data(std::string const& filename)
{
    return file_data(filename.c_str());
}

inline std::string file_data(std::wstring const& wfilename)
{
    return file_data(wfilename.c_str());
}

template <class T>
inline std::string file_data(T const& filename, std::error_code& ec)
{
    try
    {
        return std::move(file_data(filename));
    }
    catch (std::system_error const& e)
    {
        ec = e.code();
    }
    return std::string();
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string string_text(std::string const& raw)
{
    bom::type bo_raw = codec::nobomb;
    auto const cp_raw = hint_codepage(raw, bo_raw);
    if (cp == cp_raw && bo == bo_raw)
    {
        return std::move(raw);
    }
    return std::move(codec(cp, bo)(codec(cp_raw, bo_raw)(raw)));
}
 
template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string 
string_text(std::string const& raw, std::error_code& ec)
{
    try
    {
        return std::move(string_text<cp, bo>(raw));
    }
    catch (std::system_error const& e)
    {
        ec = e.code();
    }
    return std::string();
}

inline std::wstring wstring_text(std::string const& raw)
{
    bom::type bo = codec::nobomb;
    auto const cp = hint_codepage(raw, bo);
    return std::move(codec(cp, bo)(raw));
}

inline std::wstring wstring_text(std::string const& raw, std::error_code& ec)
{
    try
    {
        return std::move(wstring_text(raw));
    }
    catch (std::system_error const& e)
    {
        ec = e.code();
    }
    return std::wstring();
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string file_text(std::string const& filename)
{
    auto const text_raw = file_data(filename);
    return std::move(string_text<cp, bo>(text_raw));
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline std::string 
file_text(std::string const& filename, std::error_code& ec)
{
    auto const text_raw = file_data(filename, ec);
    return std::move(string_text<cp, bo>(text_raw, ec));
}

inline std::wstring file_text(std::wstring const& wfilename)
{
    auto const text_raw = file_data(wfilename);
    return std::move(wstring_text(text_raw));
}

inline std::wstring 
file_text(std::wstring const& wfilename, std::error_code& ec)
{
    auto const text_raw = file_data(wfilename, ec);
    return std::move(wstring_text(text_raw, ec));
}

inline size_t save_file_data(std::string const& filename
    , std::string const& data
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    std::ofstream ofile;
    ofile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        ofile.open(filename.c_str(), mode);
        ofile.write(data.data(), data.size());
        ofile.close();
    }
    catch (std::ofstream::failure const& e)
    {
        throw std::system_error(std::make_error_code
            (std::errc::no_such_file_or_directory), e.what());
    }
    return data.size();
}

inline size_t save_file_data(std::wstring const& wfilename
    , std::string const& data
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    return save_file_data(encode<codec::cp_default, codec::nobomb>(wfilename)
        , data, mode);
}

template <class T>
inline size_t save_file_data(T const& filename
    , std::string const& data
    , std::error_code& ec
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    try
    {
        return save_file_data(filename, data, mode);
    }
    catch (std::system_error const& e)
    {
        ec = e.code();
    }
    return 0U;
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline size_t save_file_text(std::string const& filename
    , std::string const& text
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    std::string const text_en = convert<codepage::cp_default, cp
        , bom::bomb, bo>(text);
    return save_file_data(filename, text_en, mode);
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline size_t save_file_text(std::string const& filename
    , std::string const& text
    , std::error_code& ec
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    try
    {
        return save_file_text<cp, bo>(filename, text, mode);
    }
    catch (std::system_error const& e)
    {
        ec = e.code();
    }
    return 0U;
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline size_t save_file_text(std::wstring const& wfilename
    , std::wstring const& wtext
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    std::string const text_en = encode<cp, bo>(wtext);
    return save_file_data(wfilename, text_en, mode);
}

template <codepage::type cp CP_DEFAULT_TEMPLATE_ARG
    , bom::type bo BOM_DEFAULT_TEMPLATE_ARG>
inline size_t save_file_text(std::wstring const& wfilename
    , std::wstring const& wtext
    , std::error_code& ec
    , std::ios_base::openmode mode = std::ios_base::out|std::ios_base::binary)
{
    try
    {
        return save_file_text<cp, bo>(wfilename, wtext, mode);
    }
    catch (std::system_error const& e)
    {
        ec = e.code();
    }
    return 0U;
}

}  // namespace una

using una::is_ascii;
using una::hint_codepage;

using una::string_text;
using una::wstring_text;

using una::file_data;
using una::save_file_data;

using una::file_text;
using una::save_file_text;

}  // namespace ymh

#endif  // YMH_UNA_HPP

