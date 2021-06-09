// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef COMMON_FEES_H
#define COMMON_FEES_H

#include <config.h>
#include <stdexcept>

namespace Common::NetworkFees
{
    /**
     * Calculate the base transaction fee given the size of the transaction in bytes
     *
     * @param transaction_size
     * @return
     */
    uint64_t calculate_base_transaction_fee(size_t transaction_size);

    /**
     * Calculate the transaction fee discount percentage given the number of leading zeros
     * found in the PoW hash of the transaction
     *
     * @param leading_zeros
     * @return
     */
    float calculate_transaction_discount(size_t leading_zeros = 0);

    /**
     * Calculate the transaction fee given the size of the transaction in bytes
     * as well as the number of leading zeros found in the PoW hash of the
     * transaction
     *
     * @param transaction_size
     * @param leading_zeros
     * @return
     */
    uint64_t calculate_transaction_fee(size_t transaction_size, size_t leading_zeros = 0);
} // namespace Common::NetworkFees

#endif // COMMON_FEES_H
