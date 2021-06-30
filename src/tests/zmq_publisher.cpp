// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_helper.h>
#include <console.h>
#include <tools/thread_helper.h>
#include <zmq_publisher.h>

using namespace Utilities;
using namespace Networking;

std::condition_variable stopping;

void auto_sender_thread(std::shared_ptr<ZMQPublisher> &server, logger &logger)
{
    while (true)
    {
        const auto hash = Crypto::random_hash();

        auto msg = zmq_message_envelope_t(hash.vector());

        msg.subject = crypto_hash_t("bf15572be229a849020316b597609fcaa30a5d0ad07048ba301d13e1ccdca90b");

        server->send(msg);

        logger->info("Sent: {0}", msg.to_string());

        if (thread_sleep(stopping, 2000))
        {
            break;
        }
    }
}

int main(int argc, char **argv)
{
    auto console = std::make_shared<ConsoleHandler>("ZMQ Test Publisher");

    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    uint16_t server_port = Configuration::Notifier::DEFAULT_BIND_PORT;

    // clang-format off
    cli->add_options("Server")
        ("p,port", "The local port to bind the server to",
            cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    cli->parse(argc, argv);

    console->catch_abort();

    auto logger = Logger::create_logger("./test-zmq-publisher.log", cli->log_level());

    auto server = std::make_shared<ZMQPublisher>(logger, server_port);

    logger->info("Starting Test ZMQ Publisher...");

    const auto error = server->bind();

    if (error)
    {
        logger->error("ZMQ Publisher could not be started: {0}", error.to_string());

        exit(1);
    }

    std::thread th(auto_sender_thread, std::ref(server), std::ref(logger));

    logger->info("Test ZMQ Publisher Started");

    console->run();

    logger->info("Test ZMQ Publisher shutting down...");

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
