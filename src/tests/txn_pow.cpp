// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <benchmark.h>
#include <cli_header.h>
#include <config.h>
#include <types.h>

#define POW_TEST_ITERATIONS 10

int main()
{
    print_cli_header();

    benchmark_header(40, 25);

    for (size_t i = 0; i < TurtleCoin::Configuration::Transaction::Fees::MAXIMUM_POW_ZEROS; ++i)
    {
        benchmark(
            [&i]() {
                TurtleCoin::Types::Blockchain::uncommited_normal_transaction_t tx;

                tx.tx_public_key = Crypto::random_point();

                [[maybe_unused]] const auto success = tx.mine(i);
            },
            "Searching for " + std::to_string(i) + " leading zeros",
            POW_TEST_ITERATIONS,
            40,
            25);
    }

    return 0;
}
