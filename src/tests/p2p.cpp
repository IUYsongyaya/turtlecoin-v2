// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <console.h>
#include <network_node.h>
#include <tools/cli_helper.h>
#include <tools/thread_helper.h>

using namespace Utilities;
using namespace P2P;

std::condition_variable stopping;

static inline void p2p_handler_thread(std::shared_ptr<NetworkNode> &server, logger &logger)
{
    while (true)
    {
        logger->info("Incoming: {0}\tOutgoing: {1}", server->incoming_connections(), server->outgoing_connections());

        if (thread_sleep(stopping, 15000))
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

    auto console = std::make_shared<ConsoleHandler>("P2P Test Service");

    console->catch_abort();

    auto logger = Logger::create_logger("./test-p2p.log", log_level);

    auto server = std::make_shared<NetworkNode>(logger, "./peerlist", server_port);

    {
        const auto error = server->start();

        if (error)
        {
            logger->error("P2P server error: {0}", error.to_string());

            exit(1);
        }
    }

    std::thread th(p2p_handler_thread, std::ref(server), std::ref(logger));

    console->run();

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
