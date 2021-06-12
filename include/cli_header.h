// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CLI_HEADER_H
#define CLI_HEADER_H

#include <ascii.h>
#include <config.h>
#include <sstream>

using namespace Configuration::Version;

static inline std::string get_cli_header()
{
    std::stringstream cli_header;

    cli_header << std::endl
               << asciiArt << std::endl
               << " " << PROJECT_NAME << " v" << MAJOR << "." << MINOR << "." << PATCH << " Build " << BUILD
               << std::endl
               << std::endl
               << " This software is distributed under the TRTL-OSLv1 License" << std::endl
               << std::endl
               << " " << COPYRIGHT << std::endl
               << std::endl
               << " Additional Copyright(s) may apply, please see the included LICENSE " << std::endl
               << " file for more information." << std::endl
               << std::endl
               << " If you did not receive a copy of the LICENSE, please visit: " << std::endl
               << " " << LICENSE_URL << std::endl
               << std::endl
               << "----------------------------------------------------------------------" << std::endl
               << std::endl;

    return cli_header.str();
}

static inline void print_cli_header()
{
    std::cout << get_cli_header() << std::flush;
}

#endif
