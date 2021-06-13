// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <address_encoding.h>
#include <crypto.h>
#include <tools/cli_helper.h>
#include <utilities.h>

using namespace Utilities;

int main(int argc, char **argv)
{
    auto options = cli_setup_options(argv);

    auto cli = cli_parse_options(argc, argv, options);

    std::cout << "Wallet Address Encoding Check" << std::endl << std::endl;

    const auto [wallet_seed, words, timestamp] = Crypto::generate_wallet_seed();

    std::cout << "Seed: " << wallet_seed << std::endl
              << std::endl
              << "Creation Timestamp: " << std::to_string(timestamp) << std::endl
              << std::endl
              << "Mnemonic Phrase: " << std::endl;

    for (size_t i = 0; i < words.size(); ++i)
    {
        if (i != 0 && i % 8 == 0)
        {
            std::cout << std::endl << "\t";
        }
        else if (i == 0)
        {
            std::cout << "\t";
        }

        std::cout << words[i];

        if (i + 1 != words.size())
        {
            std::cout << " ";
        }
    }

    std::cout << std::endl << std::endl;

    const auto [public_view, private_view] = Crypto::generate_wallet_view_keys(wallet_seed);

    std::cout << "View Keys" << std::endl
              << "\t"
              << "Private: " << private_view << std::endl
              << "\t"
              << "Public: " << public_view << std::endl
              << std::endl;

    const auto [public_spend, private_spend] = Crypto::generate_wallet_spend_keys(wallet_seed);

    std::cout << "Spend Keys" << std::endl
              << "\t"
              << "Private: " << private_spend << std::endl
              << "\t"
              << "Public: " << public_spend << std::endl
              << std::endl;

    const auto address = encode_address(public_spend, public_view);

    std::cout << "Address: " << address << std::endl << std::endl;

    {
        std::cout << "Address Decoding...: ";

        const auto [error, spend, view] = decode_address(address);

        if (error || spend != public_spend || view != public_view)
        {
            std::cout << "Failed" << std::endl;

            exit(1);
        }

        std::cout << "Passed" << std::endl << std::endl;
    }

    {
        std::cout << "v1 Address Decoding Failure...: ";

        const auto [error, spend, view] = decode_address(
            "TRTLv1QeF7jjfjnbs4nY1WMYifTnJyVpX9fosdPiP6hEJY7Mz1Z9Bfk424C6DXbebyVD5wD9prpwJQhAMMgtAzFEPVvVd9ijAk2");

        if (!error)
        {
            std::cout << "Failed" << std::endl;

            exit(1);
        }

        std::cout << "Passed" << std::endl << std::endl;
    }

    return 0;
}
