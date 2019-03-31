# Miscellaneous Skills

Miscellaneous skills for C, C++11/14/17/20, Python2/3, ECMAScript5/6 and Shell Script.

## C/C++/11/14/17/20

### [format_bytes](https://github.com/yanminhui/misc/blob/master/cpp/format_bytes.hpp)

Given a byte count, converts it to human-readable format 
and returns a string consisting of a value and a units indicator.

Depending on the size of the value, the units part is bytes, 
KB (kibibytes), MB (mebibytes), GB (gibibytes), TB (tebibytes), 
or PB (pebibytes)...

**Function Prototype**

```.cpp
template<typename CharT, typename ByteT>
CharT const* format_bytes(std::basic_string<CharT>& repr             // (1)
                        , ByteT const bytes
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u);

template<typename CharT, typename ByteT, typename IndicatorT
       , typename = typename std::enable_if<!std::is_integral<IndicatorT>::value>::type>
CharT const* format_bytes(std::basic_string<CharT>& repr             // (2)
                        , ByteT const bytes
                        , IndicatorT&& indicator
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u);

template<typename CharT, typename ByteT, typename InputIt>
CharT const* format_bytes(std::basic_string<CharT>& repr             // (3)
                        , ByteT const bytes
                        , InputIt first, InputIt last
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u);

template<typename CharT, typename ByteT
       , typename InputIt, typename IndicatorT>
CharT const* format_bytes(std::basic_string<CharT>& repr             // (4)
                        , ByteT const bytes
                        , InputIt first, InputIt last
                        , IndicatorT&& indicator
                        , std::size_t const decimal=2u
                        , std::size_t const reduced_unit=1024u);
```

**Usage**

```.cpp
using namespace ymh::misc;

std::string s;
std::cout << format_bytes(s, 18446640) << std::endl;
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

### [mcr](https://github.com/yanminhui/misc/blob/master/py/mcr.py)

Measure compression ratio. `zlib`, `gzip`, `bz2`, `lzma` is supported.

**Usage**

```bash
usage: mcr.py [-h] [--verbose] [--chunk-size {4,8,16,32,64,128,256,512}]
              [--name {gzip,all,zlib,bz2}]
              [--level {-2,-1,0,1,2,3,4,5,6,7,8,9}]
              file

Measure compression ratio.

positional arguments:
  file                  file or directory

optional arguments:
  -h, --help            show this help message and exit
  --verbose             print progress status (default: None)
  --chunk-size {4,8,16,32,64,128,256,512}
                        data chunk's size (metric: KB) (default: 16)
  --name {gzip,all,zlib,bz2}
                        compression algorithm's name (default: all)
  --level {-2,-1,0,1,2,3,4,5,6,7,8,9}
                        controlling the level of compression, all = -2,
                        default = -1 (default: -2)
```

**Example**

```bash
$ python3 mcr.py --verbose --level=-1 /usr/local
File: /usr/local, Length: 525.48 MB, Chunk Size: 16.0 KB
NAME  LEVEL OUTSIZE    EXPIRED    %SAVINGS SPEED        RATIO %PROG REMAIN
 zlib     -1 170.54 MB  42.71 secs   67.55 12.3 MBps     3.08 100.0 0.0 secs
 gzip      9 170.93 MB  57.98 secs   67.47 9.06 MBps     3.07 100.0 0.0 secs
 bz2       9 132.79 MB  1.32 mins    74.73 6.65 MBps     3.96 100.0 0.0 secs
 lzma      0 101.92 MB  8.07 mins     80.6 1.09 MBps     5.16 100.0 0.0 secs
```

## ECMAScript5/6

## Shell Script


