// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <console.h>
#include <tools/cli_helper.h>
#include <tools/thread_helper.h>
#include <zmq_server.h>

using namespace Utilities;
using namespace Networking;

std::condition_variable stopping;

void server_handler_thread(std::shared_ptr<ZMQServer> &server, logger &logger)
{
    while (true)
    {
        while (!server->messages().empty())
        {
            auto msg = server->messages().pop();

            logger->info("Received: {0}", msg.to_string());

            msg.to = msg.from;

            server->send(msg);
        }

        if (thread_sleep(stopping))
        {
            break;
        }
    }
}

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

    auto console = std::make_shared<ConsoleHandler>("ZMQ Test Server");

    console->catch_abort();

    auto logger = Logger::create_logger("./test-zmq-server.log", log_level);

    auto server = std::make_shared<ZMQServer>(logger, server_port);

    logger->info("ZMQ Server Identity: {}", server->identity().to_string());

    {
        const auto error = server->bind();

        if (error)
        {
            logger->error("ZMQ Server could not be started: {0}", error.to_string());

            exit(1);
        }
    }

    std::thread th(server_handler_thread, std::ref(server), std::ref(logger));

    console->run();

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
