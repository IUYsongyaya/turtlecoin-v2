// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CLI_HEADER_H
#define CLI_HEADER_H

#include <ascii.h>
#include <config.h>

using namespace TurtleCoin::Configuration::Version;

static inline void print_cli_header()
{
    std::cout << std::endl
              << asciiArt << std::endl
              << " " << PROJECT_NAME << " v" << MAJOR << "." << MINOR << "." << PATCH << " Build " << BUILD << std::endl
              << std::endl
              << " This software is distributed under the TRTL-OSLv1 License" << std::endl
              << std::endl
              << " " << COPYRIGHT << std::endl
              << std::endl
              << " Additional Copyright(s) may apply, please see the included LICENSE file for more information."
              << std::endl
              << std::endl
              << " If you did not receive a copy of the LICENSE, please visit: " << std::endl
              << " " << LICENSE_URL << std::endl
              << "----------------------------------------------------------------------" << std::endl
              << std::endl;
}

#endif
