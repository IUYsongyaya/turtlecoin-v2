// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <benchmark.h>
#include <types.h>

#define POW_TEST_ITERATIONS 10

int main()
{
    benchmark_header(40, 25);

    for (size_t i = 0; i < 32; ++i)
    {
        benchmark(
            [&i]() {
                TurtleCoin::Types::Blockchain::uncommited_normal_transaction_t tx;

                tx.tx_public_key = Crypto::random_point();

                const auto success = tx.mine(i);
            },
            "Searching for " + std::to_string(i) + " leading zeros",
            POW_TEST_ITERATIONS,
            40,
            25);
    }

    return 0;
}
