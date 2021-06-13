// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <tools/cli_helper.h>
#include <zmq_subscriber.h>

using namespace Networking;

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

    auto cli = cli_parse_options(argc, argv, options);

    auto client = ZMQSubscriber();

    const auto subject = crypto_hash_t("bf15572be229a849020316b597609fcaa30a5d0ad07048ba301d13e1ccdca90b");

    client.subscribe(subject);

    std::cout << "Connecting to: " << server_host << ":" << std::to_string(server_port) << "..." << std::endl
              << std::endl;

    {
        const auto error = client.connect(server_host, server_port);

        if (error)
        {
            std::cout << error << std::endl;

            exit(1);
        }
    }

    while (true)
    {
        while (!client.messages().empty())
        {
            const auto msg = client.messages().pop();

            std::cout << "Recv >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl << msg << std::endl;

            THREAD_SLEEP(50);
        }
    }
}
