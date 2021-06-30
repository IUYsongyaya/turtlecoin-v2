// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "cli_helper.h"

#include "colors.h"

#include <ascii.h>
#include <config.h>
#include <cppfs/FileHandle.h>
#include <cppfs/fs.h>
#include <credits.h>
#include <platform_folders.h>

namespace Utilities
{
    CLIHelper::CLIHelper(const std::string &path): m_options(path, ""), m_log_level(spdlog::level::level_enum::off)
    {
        // clang-format off
        m_options.add_options("")
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
    }

    cxxopts::OptionAdder CLIHelper::add_options(const std::string &group)
    {
        return m_options.add_options(group);
    }

    bool CLIHelper::argument_exists(const std::string &option)
    {
        return m_parse_result.count(option) > 0;
    }

    std::string CLIHelper::get_cli_header()
    {
        std::stringstream cli_header;

        cli_header << std::endl
                   << asciiArt << std::endl
                   << " " << Configuration::Version::PROJECT_NAME << " " << get_version() << std::endl
                   << std::endl
                   << " This software is distributed under the TRTL-OSLv1 License" << std::endl
                   << std::endl
                   << " " << Configuration::Version::COPYRIGHT << std::endl
                   << std::endl
                   << " Additional Copyright(s) may apply, please see the included LICENSE " << std::endl
                   << " file for more information." << std::endl
                   << std::endl
                   << " If you did not receive a copy of the LICENSE, please visit: " << std::endl
                   << " " << Configuration::Version::LICENSE_URL << std::endl
                   << std::endl
                   << "----------------------------------------------------------------------" << std::endl
                   << std::endl;

        return cli_header.str();
    }

    cppfs::FilePath CLIHelper::get_db_path(const std::string &directory, const std::string &name)
    {
        auto file = cppfs::fs::open(directory);

        if (!file.exists() || !file.isDirectory())
        {
            file.createDirectory();
        }

        return cppfs::FilePath(directory + "/" + name).resolved();
    }

    cppfs::FilePath CLIHelper::get_default_db_directory()
    {
        return cppfs::FilePath(get_home_directory() + "/" + Configuration::DEFAULT_DATA_DIR).resolved();
    }

    std::string CLIHelper::get_home_directory()
    {
        return cxxfolders::getDataHome();
    }

    std::string CLIHelper::get_version()
    {
        std::stringstream ss;

        ss << "v" << Configuration::Version::MAJOR << "." << Configuration::Version::MINOR << "."
           << Configuration::Version::PATCH << " (" << Configuration::Version::BUILD << ")";

        return ss.str();
    }

    spdlog::level::level_enum CLIHelper::log_level() const
    {
        return m_log_level;
    }

    void CLIHelper::parse(int argc, char **argv)
    {
        try
        {
            m_parse_result = m_options.parse(argc, argv);

            if (m_parse_result.count("help") > 0)
            {
                print_cli_header();

                std::cout << m_options.help({}) << std::endl;

                exit(0);
            }
            else if (m_parse_result.count("credits") > 0)
            {
                print_cli_header();

                std::cout << program_credits << std::endl << std::endl;

                exit(0);
            }
            else if (m_parse_result.count("version") > 0)
            {
                print_cli_header();

                exit(0);
            }

            switch (m_parse_result["log-level"].as<size_t>())
            {
                case 0:
                    m_log_level = spdlog::level::off;
                    break;
                case 1:
                    m_log_level = spdlog::level::critical;
                    break;
                case 2:
                    m_log_level = spdlog::level::err;
                    break;
                case 3:
                    m_log_level = spdlog::level::warn;
                    break;
                case 4:
                    m_log_level = spdlog::level::info;
                    break;
                case 5:
                    m_log_level = spdlog::level::debug;
                    break;
                case 6:
                    m_log_level = spdlog::level::trace;
                    break;
                default:
                    print_cli_header();

                    std::cout << m_options.help({}) << std::endl;

                    std::cout << COLOR::red << "Invalid log level specified" << COLOR::reset << std::endl << std::endl;

                    exit(1);
            }

            print_cli_header();
        }
        catch (const cxxopts::OptionException &e)
        {
            print_cli_header();

            std::cout << m_options.help({}) << std::endl;

            std::cout << COLOR::red << "Unable to parse command line argument options: " << e.what() << COLOR::reset
                      << std::endl
                      << std::endl;

            exit(1);
        }
    }

    void CLIHelper::print_cli_header()
    {
        std::cout << COLOR::green << get_cli_header() << COLOR::reset << std::flush;
    }
} // namespace Utilities
