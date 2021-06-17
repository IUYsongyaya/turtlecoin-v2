// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <tools/cli_helper.h>
#include <types.h>
#include <zmq_client.h>

using namespace Networking;
using namespace Types::Network;

int main(int argc, char **argv)
{
    std::string server_host = "127.0.0.2";

    uint16_t server_port = Configuration::P2P::DEFAULT_BIND_PORT;

    auto options = cli_setup_options(argv);

    // clang-format off
    options.add_options("Remote Host")
        ("r,remote","The remote host IP/name to connect to",
         cxxopts::value<std::string>(server_host)->default_value(server_host))
        ("p,port","The remote host port to connect to",
         cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    auto cli = cli_parse_options(argc, argv, options);

    auto client = ZMQClient();

    std::cout << "Client Identity: " << client.identity() << std::endl << std::endl;

    std::cout << "Connecting to: " << server_host << ":" << std::to_string(server_port) << "..." << std::endl
              << std::endl;

    {
        const auto error = client.connect(server_host, server_port);

        if (error)
        {
            std::cout << error << std::endl;

            exit(1);
        }
    }

    auto msg = packet_handshake_t();

    msg.peers.resize(10);

    const auto outgoing = zmq_message_envelope_t(msg.serialize());

    client.send(outgoing);

    while (true)
    {
        while (!client.messages().empty())
        {
            auto msg = client.messages().pop();

            std::cout << msg << std::endl;

            msg.to = msg.from;

            client.send(msg);
        }

        THREAD_SLEEP();
    }
}
