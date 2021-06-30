// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "utilities.h"

#include "colors.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace Utilities
{
    void print_table(const std::vector<std::tuple<std::string, std::string>> &rows)
    {
        size_t long_left = 0, long_right = 0;

        for (const auto &[left, right] : rows)
        {
            long_left = std::max(long_left, left.length());

            long_right = std::max(long_right, right.length());
        }

        size_t total_width = long_left + long_right + 7;

        std::stringstream ss;

        std::cout << COLOR::white << std::string(total_width, '=') << COLOR::reset << std::endl;

        for (const auto &[left, right] : rows)
        {
            std::cout << COLOR::white << "| " << COLOR::yellow << str_pad(left, long_left) << COLOR::white << " | "
                      << COLOR::green << str_pad(right, long_right) << COLOR::white << " |" << std::endl;
        }

        std::cout << COLOR::white << std::string(total_width, '=') << COLOR::reset << std::endl << std::endl;
    }

    std::string str_join(const std::vector<std::string> &input, const char &ch)
    {
        std::string result;

        for (const auto &part : input)
        {
            result += part + ch;
        }

        // trim the trailing character that we appended
        result = result.substr(0, result.size() - 1);

        return result;
    }

    std::string str_pad(std::string input, size_t length)
    {
        if (input.length() < length)
        {
            const auto delta = length - input.length();

            for (size_t i = 0; i < delta; ++i)
            {
                input += " ";
            }
        }

        return input;
    }

    std::vector<std::string> str_split(const std::string &input, const char &ch)
    {
        auto pos = input.find(ch);

        uint64_t initial_pos = 0;

        std::vector<std::string> result;

        while (pos != std::string::npos)
        {
            result.push_back(input.substr(initial_pos, pos - initial_pos));

            initial_pos = pos + 1;

            pos = input.find(ch, initial_pos);
        }

        result.push_back(input.substr(initial_pos, std::min(pos, input.size()) - initial_pos + 1));

        return result;
    }

    void str_trim(std::string &str, bool to_lowercase)
    {
        const auto whitespace = "\t\n\r\f\v";

        str.erase(str.find_last_not_of(whitespace) + 1);

        str.erase(0, str.find_first_not_of(whitespace));

        if (to_lowercase)
        {
            std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
        }
    }
} // namespace Utilities
