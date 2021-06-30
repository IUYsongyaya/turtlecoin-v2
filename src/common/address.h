// Copyright (c) 2020-2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef COMMON_ADDRESS_H
#define COMMON_ADDRESS_H

#include <crypto.h>
#include <errors.h>

namespace Common::Address
{
    /**
     * Decodes the public keys from a Base58 encoded public wallet address
     *
     * @param address public wallet address
     * @return
     */
    std::tuple<Error, crypto_public_key_t, crypto_public_key_t> decode(const std::string &address);

    /**
     * Encodes the public keys into a Base58 encoded public wallet address
     *
     * @param public_spend the public spend key
     * @param public_view the public view key
     * @return
     */
    std::string encode(const crypto_public_key_t &public_spend, const crypto_public_key_t &public_view);
} // namespace Common::Address

#endif // COMMON_ADDRESS_H
