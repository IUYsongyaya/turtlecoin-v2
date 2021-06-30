// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_helper.h>

int main(int argc, char **argv)
{
    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    cli->parse(argc, argv);

    return 0;
}
