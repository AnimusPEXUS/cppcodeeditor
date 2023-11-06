
#include <experimental/scope>
#include <format>
#include <iostream>
#include <mutex>

#include "utils.hpp"

namespace wayround_i2p
{
namespace codeeditor
{

    int saveStringToFile(
        std::filesystem::path pth,
        std::string           str
    )
    {
        // todo: pth must always be absolute?
        // todo: ensure directory exists. create if needed.

        int err = 0;

        auto f = std::fopen(pth.c_str(), "w");
        if (err = std::ferror(f); err != 0)
        {
            return err;
        }
        auto se02 = std::experimental::fundamentals_v3::scope_exit(
            [f]()
            {
                // todo: remove debug code
                std::cout << "scope_exit 2" << std::endl;
                std::fclose(f);
            }
        );

        return saveStringToDesc(f, str);
    }

    std::tuple<std::string, int> loadStringFromFile(
        std::filesystem::path pth
    )
    {
        return loadStringFromFile(pth, false);
    }

    std::tuple<std::string, int> loadStringFromFile(
        std::filesystem::path pth,
        bool                  allow_nonexist
    )
    {
        // todo: pth must always be absolute?
        // todo: mode tests?
        if (!std::filesystem::exists(pth))
        {
            if (allow_nonexist)
            {
                return std::tuple("", 0);
            }
            else
            {
                return std::tuple("", 1);
            }
        }

        int err = 0;

        auto f = std::fopen(pth.c_str(), "r");
        if (err = std::ferror(f); err != 0)
        {
            return std::tuple("", err);
        }
        auto se02 = std::experimental::fundamentals_v3::scope_exit(
            [f]()
            {
                // todo: remove debug code
                std::cout << "scope_exit 2" << std::endl;
                std::fclose(f);
            }
        );

        return loadStringFromDesc(f);
    }

    int saveStringToDesc(
        FILE       *f,
        std::string str
    )
    {

        int err = 0;

        auto str_c    = str.c_str();
        auto str_size = str.size();

        if (sizeof(str_c[0]) != 1)
        {
            throw std::length_error("unsupported char size: not 1");
        }

        auto offset = 0;

        while (true)
        {
            if (offset == str_size)
            {
                break;
            }
            auto length_minus_offset  = str_size - offset;
            auto count_to_write       = (size2mib <= length_minus_offset ? size2mib : length_minus_offset);
            auto writen_count         = std::fwrite(str_c + offset, sizeof(str_c[0]), count_to_write, f);
            offset                   += writen_count;
        }

        return 0;
    }

    std::tuple<std::string, int> loadStringFromDesc(
        FILE *f
    )
    {
        int         err     = 0;
        int         err_eof = 0;
        std::string txt("");
        char       *buffer = new char[size2mib];

        if (sizeof(buffer[0]) != 1)
        {
            throw std::length_error("unsupported char size: not 1");
        }

        auto se01 = std::experimental::fundamentals_v3::scope_exit(
            [buffer]()
            {
                // todo: remove debug code
                std::cout << "scope_exit 1" << std::endl;
                delete buffer;
            }
        );

        std::cout << "while" << std::endl;
        while (true)
        {
            err     = std::ferror(f);
            err_eof = std::feof(f);

            // todo: remove debug code
            std::cout << "err: " << err << " err_eof: " << err_eof << std::endl;

            if (err != 0)
            {
                return std::tuple("", err);
            }

            if (err_eof != 0)
            {
                return std::tuple(txt, 0);
            }
            auto readed = std::fread(buffer, sizeof(buffer[0]), size2mib, f);
            txt.append(std::string(buffer, readed));
            // std::cout << "while2" << std::endl;
        }
    }

    void mutexed_println(std::string s)
    {
        static std::mutex mut;
        mut.lock();
        std::cout << s << std::endl;
        mut.unlock();
    }

    int check_relpath_is_relative_and_sane(std::filesystem::path pth)
    {
        // todo: task tracker should support 'recurring todo's
        // recurring todo: make sure path sanity functions are used everywhere where needed

        if (!pth.is_relative())
        {
            return 2;
        }

        pth      = pth.lexically_normal();
        auto val = *(pth.begin());
        if (val == "." || val == "..")
        {
            return 1;
        }
        return 0;
    }

    LineStarts::LineStarts(std::string text)
    {
        auto match_end = std::sregex_iterator();
        auto nl_rex    = std::regex(R"x(\n)x");
        auto it        = std::sregex_iterator(text.begin(), text.end(), nl_rex);

        text_size = text.length();

        starts.push_back(0);

        for (;;)
        {
            if (it == match_end)
            {
                break;
            }

            auto z = *it;

            starts.push_back(z.position() + z.length());
            it++;
        }
    }

    void LineStarts::printParsingResult(std::string text)
    {
        std::cout << "LineStarts : parsing result :" << std::endl;
        {
            for (int i = 0; i != starts.size(); i++)
            {
		auto j = i + 1;
                auto info = getLineInfo(j);
                auto r0   = std::get<0>(info);
                auto r1   = std::get<1>(info);
                std::cout << format("{} ({}, {}): `{}`", j, r0, r1, text.substr(r0, r1 - r0));
            }
        }
    }

    unsigned int LineStarts::getLineByOffset(unsigned int offset)
    {

        if (offset > text_size)
        {
            return starts.size();
        }

        auto         i    = starts.begin();
        unsigned int cv   = 0;
        unsigned int nv   = 0;
        unsigned int line = 1;

        for (;;)
        {
            nv = *i;
            if (nv >= offset)
            {
                return line;
            }
            else
            {
                cv = nv;
                i++;
                line++;
            }
        }
    }

    std::tuple<
        unsigned int,
        unsigned int,
        int>
        LineStarts::getLineInfo(unsigned int index)
    {
        unsigned int start = 0;
        unsigned int end   = 0;
        int          err   = 0;

        if (index == 0)
        {
            err = 3;
            goto return_res;
        }

        index -= 1; // because line 1 saved at index 0

        if (index > starts.size() - 1)
        {
            err = 1;
            goto return_res;
        }

        if (index == 0)
        {
            start = 0;
        }
        else
        {
            start = starts[index];
        }

        if (index == starts.size() - 1)
        {
            end = text_size;
        }
        else
        {
            end = starts[index + 1];
        }

    return_res:
        return std::tuple(start, end, err);
    }

} // namespace codeeditor
} // namespace wayround_i2p
