// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <address.h>
#include <cli_helper.h>
#include <crypto.h>
#include <logger.h>
#include <utilities.h>

using namespace Common::Address;

int main(int argc, char **argv)
{
    auto cli = std::make_shared<Utilities::CLIHelper>(argv);

    cli->parse(argc, argv);

    auto logger = Logger::create_logger("", cli->log_level());

    logger->warn("Walled Address Encoding Check");

    const auto [wallet_seed, words, timestamp] = Crypto::generate_wallet_seed();

    logger->info("Seed: {0}", wallet_seed.to_string());

    logger->info("Creation Timestamp: {0}", std::to_string(timestamp));

    logger->info("Mnemonic Phrase: {0}", Utilities::str_join(words));

    const auto [public_view, private_view] = Crypto::generate_wallet_view_keys(wallet_seed);

    logger->warn("View Keys");

    logger->info("Private: {0}", private_view.to_string());

    logger->info("Public: {0}", public_view.to_string());

    const auto [public_spend, private_spend] = Crypto::generate_wallet_spend_keys(wallet_seed);

    logger->warn("Spend Keys");

    logger->info("Private: {0}", private_spend.to_string());

    logger->info("Public: {0}", public_spend.to_string());

    const auto address = encode(public_spend, public_view);

    logger->warn("Public Address");

    logger->info("Address: {0}", address);

    {
        const auto [error, spend, view] = decode(address);

        if (error || spend != public_spend || view != public_view)
        {
            logger->error("Address Decoding... Failed");

            exit(1);
        }

        logger->info("Address Decoding... Passed");
    }

    {
        const auto [error, spend, view] = decode(
            "TRTLv1QeF7jjfjnbs4nY1WMYifTnJyVpX9fosdPiP6hEJY7Mz1Z9Bfk424C6DXbebyVD5wD9prpwJQhAMMgtAzFEPVvVd9ijAk2");

        if (!error)
        {
            logger->error("v1 Address Decoding Failure... Failed");

            exit(1);
        }

        logger->info("v1 Address Decoding Failure... Passed");
    }

    return 0;
}
