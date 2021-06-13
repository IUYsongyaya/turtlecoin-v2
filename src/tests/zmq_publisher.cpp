// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <tools/cli_helper.h>
#include <zmq_publisher.h>

using namespace Networking;

int main(int argc, char **argv)
{
    uint16_t server_port = Configuration::Notifier::DEFAULT_BIND_PORT;

    auto options = cli_setup_options(argv);

    // clang-format off
    options.add_options("Server")
        ("p,port", "The local port to bind the server to",
            cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)));
    // clang-format on

    auto cli = cli_parse_options(argc, argv, options);

    auto server = ZMQPublisher(server_port);

    std::cout << "Binding server to: *:" << std::to_string(server_port) << "..." << std::endl << std::endl;

    const auto error = server.bind();

    if (error)
    {
        std::cout << error << std::endl;

        exit(1);
    }

    while (true)
    {
        const auto hash = Crypto::random_hash();

        auto msg = zmq_message_envelope_t(hash.vector());

        msg.subject = crypto_hash_t("bf15572be229a849020316b597609fcaa30a5d0ad07048ba301d13e1ccdca90b");

        server.send(msg);

        std::cout << "Sent >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << std::endl << msg << std::endl;

        THREAD_SLEEP(2000);
    }
}
