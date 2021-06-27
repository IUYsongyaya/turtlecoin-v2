// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "utilities.h"

namespace Utilities
{
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
