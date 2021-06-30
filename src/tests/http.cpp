// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_helper.h>
#include <console.h>
#include <http_client.h>
#include <http_server.h>

using namespace Utilities;
using namespace Networking;

int main(int argc, char **argv)
{
    auto console = std::make_shared<ConsoleHandler>("HTTP Server Test");

    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    uint16_t server_port = Configuration::API::DEFAULT_NODE_BIND_PORT;

    size_t server_timeout = 30;

    // clang-format off
    cli->add_options("Server")
        ("p,port", "The local port to bind the server to",
         cxxopts::value<uint16_t>(server_port)->default_value(std::to_string(server_port)))
        ("t,timeout", "Keep the test server running for N seconds",
         cxxopts::value<size_t>(server_timeout)->default_value(std::to_string(server_timeout)));
    // clang-format on

    cli->parse(argc, argv);

    console->catch_abort();

    auto logger = Logger::create_logger("./test-http.log", cli->log_level());

    auto server = std::make_shared<HTTPServer>(logger);

    server->Get(
        "/",
        [](const auto &request, auto &response)
        {
            auto sig = crypto_clsag_signature_t();

            JSON_INIT_BUFFER(buffer, writer);

            sig.toJSON(writer);

            JSON_DUMP_BUFFER(buffer, result);

            return response.set_content(result, "application/json");
        });

    logger->info("HTTP Test server starting...");

    if (!server->listen("0.0.0.0", server_port))
    {
        logger->error("HTTP server could not be started");

        exit(1);
    }

    auto client = HTTPClient::create_client("127.0.0.1", server_port);

    auto res = client->Get("/");

    if (res->status == 200)
    {
        const auto [error, json] = HTTPClient::parse_json_body(res, HTTP_BODY_REQUIRED);

        if (!error)
        {
            logger->info("Client received valid JSON: {}", res->body);
        }
        else
        {
            logger->error("Client JSON parsing error: {}", error.to_string());

            exit(1);
        }
    }
    else
    {
        logger->error("Client received unexpected HTTP status code from server: {}", res->status);

        exit(1);
    }

    logger->info("HTTP Test Server Started");

    console->run();

    logger->info("HTTP Tester Server shutting down");
}
