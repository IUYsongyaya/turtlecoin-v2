// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_UTILITIES_CLI_H
#define TURTLECOIN_UTILITIES_CLI_H

#include <cppfs/FilePath.h>
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

namespace Utilities
{
    class CLIHelper
    {
      public:
        CLIHelper(const std::string &path);

        CLIHelper(char **argv): CLIHelper(argv[0]) {};

        cxxopts::OptionAdder add_options(const std::string &group = "");

        bool argument_exists(const std::string &option);

        template<typename type> void argument_load(const std::string &option, type &target)
        {
            if (argument_exists(option))
            {
                target = m_parse_result[option].as<type>();
            }
        }

        template<typename type> type argument_value(const std::string &option)
        {
            return m_parse_result[option].as<type>();
        }

        static std::string get_cli_header();

        static cppfs::FilePath get_db_path(const std::string &directory, const std::string &name);

        static cppfs::FilePath get_default_db_directory();

        static std::string get_home_directory();

        static std::string get_version();

        [[nodiscard]] spdlog::level::level_enum log_level() const;

        void parse(int argc, char **argv);

        static void print_cli_header();

      private:
        cxxopts::Options m_options;

        spdlog::level::level_enum m_log_level;

        cxxopts::ParseResult m_parse_result;
    };
} // namespace Utilities

#endif
