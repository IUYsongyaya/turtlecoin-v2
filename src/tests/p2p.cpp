// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <network_node.h>
#include <tools/cli_helper.h>

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

    auto logger = Logger::create_logger("./test-p2p.log");

    auto server = P2P::NetworkNode(logger, "./peerlist", server_port);

    {
        const auto error = server.start();

        if (error)
        {
            std::cout << error << std::endl;

            exit(1);
        }
    }

    while (server.running())
    {
        std::cout << "Outgoing: " << server.outgoing_connections() << std::endl
                  << "Incoming: " << server.incoming_connections() << std::endl
                  << std::endl;

        THREAD_SLEEP_MS(15000);
    }

    std::cout << "Normal Exit" << std::endl;

    return 0;
}
