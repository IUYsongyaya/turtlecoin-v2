// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_helper.h>
#include <console.h>
#include <p2p_node.h>
#include <tools/thread_helper.h>

using namespace Utilities;
using namespace P2P;

std::condition_variable stopping;

static inline void p2p_handler_thread(std::shared_ptr<Node> &server, logger &logger)
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
    auto console = std::make_shared<ConsoleHandler>("P2P Test Service");

    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    uint16_t server_port = Configuration::P2P::DEFAULT_BIND_PORT;

    auto seed_nodes = std::vector<std::string>();

    // clang-format off
    cli->add_options("Server")
        ("p,port", "The local port to bind the server to",
            cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)))
        ("seed-node", "Additional seed nodes to attempt when bootstrapping",
            cxxopts::value<std::vector<std::string>>(seed_nodes), "<ip:port>");
    // clang-format on

    cli->parse(argc, argv);

    console->catch_abort();

    auto logger = Logger::create_logger("./test-p2p.log", cli->log_level());

    auto server = std::make_shared<Node>(logger, "./peerlist", server_port);

    logger->info("Starting Test P2P Node...");

    {
        const auto error = server->start(seed_nodes);

        if (error)
        {
            logger->error("Test P2P Node could not start: {0}", error.to_string());

            exit(1);
        }
    }

    std::thread th(p2p_handler_thread, std::ref(server), std::ref(logger));

    logger->info("P2P Node started on *:{0}", server_port);

    console->run();

    logger->info("P2P Node shutting down...");

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
