// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CLI_HEADER_H
#define CLI_HEADER_H

#include "../credits.h"

#include <ascii.h>
#include <colors.h>
#include <config.h>
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>
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
    std::cout << COLOR::green << get_cli_header() << COLOR::reset << std::flush;
}

static inline cxxopts::Options cli_setup_options(const std::string &path)
{
    cxxopts::Options options(path, "");

    // clang-format off
    options.add_options("")
        ("credits", "Display a full listing of the program credits",
            cxxopts::value<bool>()->implicit_value("true"))
        ("h,help", "Display this help message",
            cxxopts::value<bool>()->implicit_value("true"))
        ("log-file", "Specify the <path> to the log file",
            cxxopts::value<std::string>(), "<path>")
        ("log-level", "Sets the default logging level (0-6)",
            cxxopts::value<size_t>()->default_value(std::to_string(Configuration::DEFAULT_LOG_LEVEL)), "#")
        ("v,version", "Display the software version information",
            cxxopts::value<bool>()->implicit_value("true"));
    // clang-format on

    return options;
}

static inline cxxopts::Options cli_setup_options(char **argv)
{
    return cli_setup_options(argv[0]);
}

static inline std::tuple<cxxopts::ParseResult, spdlog::level::level_enum>
    cli_parse_options(int argc, char **argv, cxxopts::Options &options)
{
    try
    {
        auto result = options.parse(argc, argv);

        if (result.count("help") > 0)
        {
            print_cli_header();

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

        spdlog::level::level_enum log_level;

        switch (result["log-level"].as<size_t>())
        {
            case 0:
                log_level = spdlog::level::off;
                break;
            case 1:
                log_level = spdlog::level::critical;
                break;
            case 2:
                log_level = spdlog::level::err;
                break;
            case 3:
                log_level = spdlog::level::warn;
                break;
            case 4:
                log_level = spdlog::level::info;
                break;
            case 5:
                log_level = spdlog::level::debug;
                break;
            case 6:
                log_level = spdlog::level::trace;
                break;
            default:
                print_cli_header();

                std::cout << options.help({}) << std::endl;

                std::cout << COLOR::red << "Invalid log level specified" << COLOR::reset << std::endl << std::endl;

                exit(1);
        }

        print_cli_header();

        return {result, log_level};
    }
    catch (const cxxopts::OptionException &e)
    {
        print_cli_header();

        std::cout << options.help({}) << std::endl;

        std::cout << COLOR::red << "Unable to parse command line argument options: " << e.what() << COLOR::reset
                  << std::endl
                  << std::endl;

        exit(1);
    }
}

#endif
