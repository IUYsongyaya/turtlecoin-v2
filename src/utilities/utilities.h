// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_UTILITIES_H
#define TURTLECOIN_UTILITIES_H

#include <string>
#include <vector>

namespace Utilities
{
    /**
     * Prints the given tuple of left/right columns as a table
     * @param rows
     */
    void print_table(const std::vector<std::tuple<std::string, std::string>> &rows);

    /**
     * Joins a vector of strings together using the specified character as the delimiter
     *
     * @param input
     * @param ch
     * @return
     */
    std::string str_join(const std::vector<std::string> &input, const char &ch = ' ');

    /**
     * Pads a string with blank spaces up to the specified length
     *
     * @param input
     * @param length
     * @return
     */
    std::string str_pad(std::string input, size_t length = 0);

    /**
     * Splits a string into a vector of strings using the specified character as a delimiter
     *
     * @param input
     * @param ch
     * @return
     */
    std::vector<std::string> str_split(const std::string &input, const char &ch = ' ');

    /**
     * Trims any whitespace from both the start and end of the given string
     *
     * @param str
     * @param to_lowercase
     */
    void str_trim(std::string &str, bool to_lowercase = false);
} // namespace Utilities

#endif // TURTLECOIN_UTILITIES_H
