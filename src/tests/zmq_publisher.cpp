// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <console.h>
#include <tools/cli_helper.h>
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
    uint16_t server_port = Configuration::Notifier::DEFAULT_BIND_PORT;

    auto options = cli_setup_options(argv);

    // clang-format off
    options.add_options("Server")
        ("p,port", "The local port to bind the server to",
            cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    auto [cli, log_level] = cli_parse_options(argc, argv, options);

    auto console = std::make_shared<ConsoleHandler>("ZMQ Test Publisher");

    console->catch_abort();

    auto logger = Logger::create_logger("./test-zmq-publisher.log", log_level);

    auto server = std::make_shared<ZMQPublisher>(logger, server_port);

    const auto error = server->bind();

    if (error)
    {
        logger->error("ZMQ Publisher could not be started: {0}", error.to_string());

        exit(1);
    }

    std::thread th(auto_sender_thread, std::ref(server), std::ref(logger));

    console->run();

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
