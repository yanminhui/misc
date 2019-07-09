#include <cstdio>

#include "una.hpp"

int main()
{
    /*
    std::locale loc("");
    std::locale::global(loc);
    std::wcout.imbue(loc);
    */
    try
    {
        auto raw = u8"中華人民共和國，福建省廈門市，軟件園二期";
        auto uni = ymh::UTF8ToUnicode(raw);
        auto ansi = ymh::UnicodeToANSI(uni);
        auto res = ansi;
        puts("Result: ");
        puts(res.c_str());
    } 
    catch (std::exception const& e)
    {
        puts("Error: ");
            puts(e.what());
        return 1;
    }
    return 0;
}

