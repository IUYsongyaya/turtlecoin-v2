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

    auto [cli, log_level] = cli_parse_options(argc, argv, options);

    auto logger = Logger::create_logger("./test-zmq-server.log", log_level);

    auto server = std::make_shared<ZMQServer>(logger, server_port);

    logger->info("ZMQ Server Identity: {}", server->identity().to_string());

    {
        const auto error = server->bind();

        if (error)
        {
            logger->error("ZMQ Server could not be started: {}", error.to_string());

            exit(1);
        }
    }

    while (true)
    {
        while (!server->messages().empty())
        {
            auto msg = server->messages().pop();

            std::cout << msg << std::endl;

            msg.to = msg.from;

            server->send(msg);
        }

        THREAD_SLEEP();
    }
}
