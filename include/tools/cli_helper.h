// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CLI_HEADER_H
#define CLI_HEADER_H

#include "../credits.h"

#include <ascii.h>
#include <config.h>
#include <cxxopts.hpp>
#include <sstream>

namespace config = Configuration::Version;

static inline std::string get_cli_header()
{
    std::stringstream cli_header;

    cli_header << std::endl
               << asciiArt << std::endl
               << " " << config::PROJECT_NAME << " v" << config::MAJOR << "." << config::MINOR << "." << config::PATCH
               << " Build " << config::BUILD << std::endl
               << std::endl
               << " This software is distributed under the TRTL-OSLv1 License" << std::endl
               << std::endl
               << " " << config::COPYRIGHT << std::endl
               << std::endl
               << " Additional Copyright(s) may apply, please see the included LICENSE " << std::endl
               << " file for more information." << std::endl
               << std::endl
               << " If you did not receive a copy of the LICENSE, please visit: " << std::endl
               << " " << config::LICENSE_URL << std::endl
               << std::endl
               << "----------------------------------------------------------------------" << std::endl
               << std::endl;

    return cli_header.str();
}

static inline void print_cli_header()
{
    std::cout << get_cli_header() << std::flush;
}

static inline cxxopts::Options cli_setup_options(const std::string &path)
{
    cxxopts::Options options(path, get_cli_header());

    // clang-format off
    options.add_options("")
        ("h,help", "Display this help message", cxxopts::value<bool>()->implicit_value("true"))
        ("credits", "Display a full listing of the program credits", cxxopts::value<bool>()->implicit_value("true"))
        ("v,version", "Display the software version information", cxxopts::value<bool>()->implicit_value("true"));
    // clang-format on

    return options;
}

static inline cxxopts::Options cli_setup_options(char **argv)
{
    return cli_setup_options(argv[0]);
}

static inline cxxopts::ParseResult cli_parse_options(int argc, char **argv, cxxopts::Options &options)
{
    try
    {
        auto result = options.parse(argc, argv);

        if (result.count("help") > 0)
        {
            std::cout << options.help({}) << std::endl;

            exit(0);
        }
        else if (result.count("credits") > 0)
        {
            print_cli_header();

            std::cout << program_credits << std::endl << std::endl;

            exit(0);
        }
        else if (result.count("version") > 0)
        {
            print_cli_header();

            exit(0);
        }

        print_cli_header();

        return result;
    }
    catch (const cxxopts::OptionException &e)
    {
        std::cout << "Error: Unable to parse command line argument options: " << e.what() << std::endl << std::endl;

        std::cout << options.help({}) << std::endl;

        exit(1);
    }
}

#endif
