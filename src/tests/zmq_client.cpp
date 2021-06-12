// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_header.h>
#include <types.h>
#include <zmq_client.h>

using namespace Networking;
using namespace Types::Network;

int main()
{
    print_cli_header();

    auto client = ZMQClient();

    std::cout << "Client Identity: " << client.identity() << std::endl << std::endl;

    {
        const auto error = client.connect("127.0.0.2", Configuration::P2P::DEFAULT_BIND_PORT);

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
    }
}
