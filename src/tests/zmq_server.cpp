// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_helper.h>
#include <console.h>
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
    auto console = std::make_shared<ConsoleHandler>("ZMQ Test Server");

    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    uint16_t server_port = Configuration::P2P::DEFAULT_BIND_PORT;

    // clang-format off
    cli->add_options("Server")
        ("p,port", "The local port to bind the server to",
         cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    cli->parse(argc, argv);

    console->catch_abort();

    auto logger = Logger::create_logger("./test-zmq-server.log", cli->log_level());

    auto server = std::make_shared<ZMQServer>(logger, server_port);

    logger->info("ZMQ Server Identity: {}", server->identity().to_string());

    logger->info("Test ZMQ Server Starting...");

    {
        const auto error = server->bind();

        if (error)
        {
            logger->error("ZMQ Server could not be started: {0}", error.to_string());

            exit(1);
        }
    }

    std::thread th(server_handler_thread, std::ref(server), std::ref(logger));

    logger->info("Test ZMQ Server Started");

    console->run();

    logger->info("Test ZMQ Server shutting down...");

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
