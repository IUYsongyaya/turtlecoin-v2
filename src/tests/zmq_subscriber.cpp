// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <console.h>
#include <tools/cli_helper.h>
#include <tools/thread_helper.h>
#include <zmq_subscriber.h>

using namespace Utilities;
using namespace Networking;

std::condition_variable stopping;

void client_handler_thread(std::shared_ptr<ZMQSubscriber> &client, logger &logger)
{
    while (true)
    {
        while (!client->messages().empty())
        {
            const auto msg = client->messages().pop();

            logger->info("Received: {0}", msg.to_string());
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

    uint16_t server_port = Configuration::Notifier::DEFAULT_BIND_PORT;

    auto options = cli_setup_options(argv);

    // clang-format off
    options.add_options("Remote Host")
        ("r,remote","The remote host IP/name to connect to",
         cxxopts::value<std::string>(server_host)->default_value(server_host))
        ("p,port","The remote host port to connect to",
         cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    auto [cli, log_level] = cli_parse_options(argc, argv, options);

    auto logger = Logger::create_logger("./test-zmq-subscriber.log", log_level);

    auto client = std::make_shared<ZMQSubscriber>(logger);

    const auto subject = crypto_hash_t("bf15572be229a849020316b597609fcaa30a5d0ad07048ba301d13e1ccdca90b");

    client->subscribe(subject);

    {
        const auto error = client->connect(server_host, server_port);

        if (error)
        {
            logger->error("ZMQ Subscriber connection error: {0}", error.to_string());

            exit(1);
        }
    }

    std::thread th(client_handler_thread, std::ref(client), std::ref(logger));

    auto console = std::make_shared<ConsoleHandler>("ZMQ Test Subscriber");

    console->run();

    stopping.notify_all();

    if (th.joinable())
    {
        th.join();
    }
}
