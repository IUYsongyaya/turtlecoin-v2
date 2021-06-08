// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "network_fees.h"

#include <cmath>

namespace TurtleCoin::Common::NetworkFees
{
    uint64_t calculate_base_transaction_fee(size_t transaction_size)
    {
        // If less than or equal to the "free" zone return the minimum
        if (transaction_size <= TurtleCoin::Configuration::Transaction::Fees::BASE_CHUNK_SIZE)
        {
            return TurtleCoin::Configuration::Transaction::Fees::MINIMUM_FEE;
        }

        // subtract the free zone
        transaction_size -= TurtleCoin::Configuration::Transaction::Fees::BASE_CHUNK_SIZE;

        // calculate the number of chunks required and round up
        const auto chunks = uint64_t(ceil(transaction_size / TurtleCoin::Configuration::Transaction::Fees::CHUNK_SIZE));

        // calculate the cost of the non-free chunks
        const auto chunk_cost = chunks * TurtleCoin::Configuration::Transaction::Fees::CHUNK_FEE;

        // base fee is the chunk cost plus the minimum fee
        return chunk_cost + TurtleCoin::Configuration::Transaction::Fees::MINIMUM_FEE;
    }

    float calculate_transaction_discount(size_t leading_zeros)
    {
        // prevent out of range issues
        if (leading_zeros > 256 || leading_zeros < 0)
        {
            throw std::range_error("leading_zeros value out of range");
        }

        /**
         * If the number of leading zeros is less than or equal to the minimum
         * number of leading zeros required then there is no discount
         */
        if (leading_zeros <= TurtleCoin::Configuration::Transaction::Fees::MINIMUM_POW_ZEROS)
        {
            return 0;
        }

        /**
         * If the number of leading zeros is more than or equal to the maximum
         * number of leading zeros permitted then the discount is the maximum
         */
        if (leading_zeros >= TurtleCoin::Configuration::Transaction::Fees::MAXIMUM_POW_ZEROS)
        {
            return ((TurtleCoin::Configuration::Transaction::Fees::MAXIMUM_POW_ZEROS
                     - TurtleCoin::Configuration::Transaction::Fees::MINIMUM_POW_ZEROS)
                    * TurtleCoin::Configuration::Transaction::Fees::POW_ZERO_DISCOUNT_MULTIPLIER)
                   / 100;
        }

        /**
         * Our discount percentage is the the lessor of the number of leading zeros minus
         * the minimum zeros required or the maximum fee discount zeros
         */
        return ((float(leading_zeros) - TurtleCoin::Configuration::Transaction::Fees::MINIMUM_POW_ZEROS)
                * TurtleCoin::Configuration::Transaction::Fees::POW_ZERO_DISCOUNT_MULTIPLIER)
               / 100;
    }

    uint64_t calculate_transaction_fee(size_t transaction_size, size_t leading_zeros)
    {
        // prevent out of range issues
        if (leading_zeros > 256 || leading_zeros < 0)
        {
            throw std::range_error("leading_zeros value out of range");
        }

        // calculate the base transaction fee
        const auto base_transaction_fee = calculate_base_transaction_fee(transaction_size);

        // get our transaction discount
        const auto discount_percentage = calculate_transaction_discount(leading_zeros);

        // determine the resulting fee discount
        const auto fee_discount = uint64_t(floor(base_transaction_fee * discount_percentage));

        // determine the discounted fee
        const auto discounted_fee = base_transaction_fee - fee_discount;

        /**
         * If the discounted fee exceeds the minimum network fee then we can use the discounted fee
         * otherwise, we need to use the minimum network fee
         */
        return std::max(discounted_fee, TurtleCoin::Configuration::Transaction::Fees::MINIMUM_FEE);
    }
} // namespace TurtleCoin::Common::NetworkFees
