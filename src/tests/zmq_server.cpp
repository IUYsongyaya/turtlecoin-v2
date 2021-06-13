// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <tools/cli_helper.h>
#include <zmq_server.h>

using namespace Networking;

int main(int argc, char **argv)
{
    uint16_t server_port = Configuration::P2P::DEFAULT_BIND_PORT;

    auto options = cli_setup_options(argv);

    // clang-format off
    options.add_options("Server")
        ("p,port", "The local port to bind the server to",
         cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    auto cli = cli_parse_options(argc, argv, options);

    auto server = ZMQServer(server_port);

    std::cout << "Server Identity: " << server.identity() << std::endl << std::endl;

    std::cout << "Binding server to: *:" << std::to_string(server_port) << "..." << std::endl << std::endl;

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
