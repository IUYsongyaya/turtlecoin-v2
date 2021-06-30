// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_helper.h>
#include <console.h>
#include <p2p_node.h>

int main(int argc, char **argv)
{
    auto console = std::make_shared<Utilities::ConsoleHandler>(Configuration::Version::PROJECT_NAME + " Seed Node");

    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    uint16_t server_port = Configuration::P2P::DEFAULT_BIND_PORT;

    auto seed_nodes = std::vector<std::string>();

    const auto default_db_path = cli->get_default_db_directory();

    std::string db_path = default_db_path.toNative(), log_path;

    // clang-format off
    cli->add_options("Seed Node")
        ("d,db-path", "Specify the <path> to the database directory",
            cxxopts::value<std::string>(db_path)->default_value(db_path), "<path>")
        ("p,port", "The local port to bind the server to",
            cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)), "#")
        ("seed-node", "Additional seed nodes to attempt when bootstrapping",
            cxxopts::value<std::vector<std::string>>(seed_nodes), "<ip:port>");
    // clang-format on

    cli->parse(argc, argv);

    cli->argument_load("log-file", log_path);

    const auto database_path = cli->get_db_path(db_path, "peerlist");

    console->catch_abort();

    auto logger = Logger::create_logger(log_path, cli->log_level());

    auto server = std::make_shared<P2P::Node>(logger, database_path.path(), server_port, true);

    console->register_command(
        "status",
        "Displays the current node status",
        [&]()
        {
            std::vector<std::tuple<std::string, std::string>> rows {
                {"Version", cli->get_version()},
                {"P2P Version", std::to_string(Configuration::P2P::VERSION)},
                {"Minimum P2P Version", std::to_string(Configuration::P2P::MINIMUM_VERSION)},
                {"Peer ID", server->peer_id().to_string()},
                {"Incoming Connections", std::to_string(server->incoming_connections())},
                {"Outgoing Connections", std::to_string(server->outgoing_connections())},
                {"Known Peers", std::to_string(server->peers()->count())}};

            Utilities::print_table(rows);
        });

    console->register_command(
        "peers",
        "Prints the full list of known peers",
        [&]()
        {
            const auto peers = server->peers()->peers();

            if (!peers.empty())
            {
                std::vector<std::tuple<std::string, std::string>> rows;

                rows.reserve(peers.size());

                for (const auto &peer : peers)
                {
                    rows.emplace_back(
                        peer.address.to_string() + ":" + std::to_string(peer.port), std::to_string(peer.last_seen));
                }

                Utilities::print_table(rows);
            }
            else
            {
                logger->info("Peer list is empty");
            }
        });

    console->register_command(
        "prune_peers",
        "Performs a pruning of our peer list",
        [&]()
        {
            server->peers()->prune();

            logger->info("Pruned peer list");
        });

    logger->info("Starting seed node...");

    const auto error = server->start(seed_nodes);

    if (error)
    {
        logger->error("Seed node could not start: {0}", error.to_string());

        exit(1);
    }

    logger->info("P2P Seed node started on *:{0}", server_port);

    console->run();

    logger->info("P2P Seed node shutting down");
}
