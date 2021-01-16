// Copyright (c) 2020-2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "address_encoding.h"

#include <config.h>

namespace TurtleCoin::Utilities
{
    std::tuple<bool, crypto_public_key_t, crypto_public_key_t> decode_address(const std::string &address)
    {
        auto [success, decoded] = Crypto::CNBase58::decode_check(address);

        if (!success)
        {
            return {false, {}, {}};
        }

        // extract the public keys from the result
        try
        {
            const auto prefix = decoded.varint<uint64_t>();

            if (prefix != TurtleCoin::Configuration::PUBLIC_ADDRESS_PREFIX)
            {
                return {false, {}, {}};
            }

            const auto public_spend = decoded.key<crypto_public_key_t>();

            const auto public_view = decoded.key<crypto_public_key_t>();

            if (!public_spend.check() || !public_view.check())
            {
                return {false, {}, {}};
            }

            return {true, public_spend, public_view};
        }
        catch (...)
        {
            return {false, {}, {}};
        }
    }

    std::string encode_address(const crypto_public_key_t &public_spend, const crypto_public_key_t &public_view)
    {
        // construct the raw address [prefix || public_spend || public_view]
        serializer_t writer;

        writer.varint(TurtleCoin::Configuration::PUBLIC_ADDRESS_PREFIX);

        writer.key(public_spend);

        writer.key(public_view);

        // Encode the raw address as Base58 and prepend the Base58 encoded prefix
        return Crypto::CNBase58::encode_check(writer);
    }
} // namespace TurtleCoin::Utilities
