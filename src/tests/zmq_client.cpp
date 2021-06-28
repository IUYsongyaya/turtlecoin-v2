// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <console.h>
#include <tools/cli_helper.h>
#include <tools/thread_helper.h>
#include <types.h>
#include <zmq_client.h>

using namespace Utilities;
using namespace Networking;
using namespace Types::Network;

std::condition_variable stopping;

void client_handler_thread(std::shared_ptr<ZMQClient> &client, logger &logger)
{
    while (true)
    {
        while (!client->messages().empty())
        {
            auto msg = client->messages().pop();

            logger->info("Received: {0}", msg.to_string());

            msg.to = msg.from;

            client->send(msg);
        }

        if (thread_sleep(stopping))
        {
            break;
        }
    }
}

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

    auto [cli, log_level] = cli_parse_options(argc, argv, options);

    auto logger = Logger::create_logger("./test-zmq-client.log", log_level);

    auto console = std::make_shared<ConsoleHandler>("ZMQ Test Client");

    console->catch_abort();

    auto client = std::make_shared<ZMQClient>(logger);

    logger->info("ZMQ Client Identity: {0}", client->identity().to_string());

    {
        const auto error = client->connect(server_host, server_port);

        if (error)
        {
            logger->error("ZMQ Client connection error: {0}", error.to_string());

            exit(1);
        }
    }

    auto msg = packet_handshake_t();

    msg.peers.resize(10);

    const auto outgoing = zmq_message_envelope_t(msg.serialize());

    client->send(outgoing);

    std::thread th(client_handler_thread, std::ref(client), std::ref(logger));

    console->run();

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
