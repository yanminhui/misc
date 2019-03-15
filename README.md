# Miscellaneous Skills

Miscellaneous skills for C, C++11/14/17/20, Python2/3, ECMAScript5/6 and Shell Script.

## C/C++/11/14/17/20

### format_bytes

**Function Prototype**

```.cpp
template<typename CharT, typename ByteT>
void format_bytes(std::basic_string<CharT>& repr                     // (1)
                 , ByteT const bytes
                 , std::size_t const decimal=2u
                 , std::size_t const reduced_unit=1024u);

template<typename CharT, typename ByteT>
void format_bytes(std::basic_string<CharT>& repr                     // (2)
                , ByteT const bytes
                , std::basic_string<CharT> const& indicator
                , std::size_t const decimal=2u
                , std::size_t const reduced_unit=1024u);

template<typename CharT, typename ByteT, typename InputIt>
void format_bytes(std::basic_string<CharT>& repr                     // (3)
                , ByteT const bytes
                , InputIt first, InputIt last
                , std::size_t const decimal=2u
                , std::size_t const reduced_unit=1024u);

template<typename CharT, typename ByteT, typename InputIt>
void format_bytes(std::basic_string<CharT>& repr                     // (4)
                , ByteT const bytes
                , InputIt first, InputIt last
                , std::basic_string<CharT> const& indicator
                , std::size_t const decimal=2u
                , std::size_t const reduced_unit=1024u);
 ```

**Usage**

```.cpp
using namespace ymh::misc;

std::string s;
format_bytes(s, 18446640);
```

equal to:

```.cpp
std::wstring wcs;  // unicode
auto indicators = { "Bytes", "KB", "MB", "GB" };
format_bytes(s, 18446640
           , std::begin(indicators), std::end(indicators)
           , "MB", 2u, 1024u);
```

**Output**

```.sh
17.60 MB
```

## Python2/3

## ECMAScript5/6

## Shell Script


