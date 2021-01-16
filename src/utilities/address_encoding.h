// Copyright (c) 2020-2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_BASE58_H
#define TURTLECOIN_BASE58_H

#include <crypto.h>

namespace TurtleCoin::Utilities
{
    /**
     * Decodes the public keys from a Base58 encoded public wallet address
     * @param address public wallet address
     * @return
     */
    std::tuple<bool, crypto_public_key_t, crypto_public_key_t> decode_address(const std::string &address);

    /**
     * Encodes the public keys into a Base58 encoded public wallet address
     * @param public_spend the public spend key
     * @param public_view the public view key
     * @return
     */
    std::string encode_address(const crypto_public_key_t &public_spend, const crypto_public_key_t &public_view);
} // namespace TurtleCoin::Utilities

#endif // TURTLECOIN_BASE58_H
