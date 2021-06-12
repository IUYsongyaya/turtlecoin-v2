// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_header.h>
#include <zmq_server.h>

using namespace Networking;

int main()
{
    print_cli_header();

    auto server = ZMQServer();

    std::cout << "Server Identity: " << server.identity() << std::endl << std::endl;

    {
        const auto error = server.bind();

        if (error)
        {
            std::cout << error << std::endl;

            exit(1);
        }
    }

    while (true)
    {
        while (!server.messages().empty())
        {
            auto msg = server.messages().pop();

            std::cout << msg << std::endl;

            msg.to = msg.from;

            server.send(msg);
        }
    }
}
