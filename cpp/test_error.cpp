#include <fstream>
#include <iostream>
#include <string>

/* Include boost.system before error.hpp 
 * if want to use:
 * 	- error_t::set_error_code(boost::system::error_code)
 * 	- error_t::make_error_code(boost::system::errc::errc_t)
 * 	- SET_ERROR_CODE()
 * 	- MAKE_ERROR_CODE()
 */
// #include <boost/system/error_code.hpp>
#include "error.hpp"

struct MyFile
{
    bool Open(std::string const& filename)
    {
        std::ofstream ofile(filename, std::ios_base::in);
        return ofile ? true : false;
    }
};

struct FileOp
{
    std::string Read(std::string const& filename, ymh::error_t& e)
    {
        MyFile mf;
        if (!mf.Open(filename))
        {
            MAKE_ERROR_CODE(e, std::errc::no_such_file_or_directory);
            return "";
        }
        return "<data-from-file>";
    }

    void Save(std::string const& filename, std::string const& data, ymh::error_t& e)
    {
        MyFile mf;
        if (!mf.Open(filename))
        {
            MAKE_ERROR_CODE(e, std::errc::bad_file_descriptor);
            return;
        }
        // Save To File.
        return;
    }

    void Merge(std::string const& to, std::string const& from, ymh::error_t& e)
    {
        auto f = Read(from, e);
        if (e)
        {
            SET_ERROR_STRING(e, "Read %s fatal", from.c_str());
            return;
        }
        auto t = Read(to, e);
        if (e)
        {
            SET_ERROR_STRING(e, "Read %s except", to.c_str());
            return;
        }
        Save(to, t + f, e);
        if (e)
        {
            SET_ERROR_STRING(e, "Save %s error", to.c_str());
            return;
        }
    }
};

bool Foo(ymh::error_t& e)
{
    auto const to = "./to.txt";
    auto const from = "./from.txt";

    FileOp fo;
    fo.Merge(to, from, e);
    if (e)
    {
        SET_ERROR_STRING(e, "Merge %s to %s failed", from, to);
        return false;
    }
    return true;
}

int main()
{
    ymh::error_t e;
    if (!Foo(e))
    {
        SET_SYSTEM_ERROR(e, errno);  // GetLastError()
        e.dump_backtrace(std::cout);

        std::cout << "EXIT_FAILURE: " << e.value() << "-" << e.message() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

