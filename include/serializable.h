// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_SERIALIZABLE_H
#define TURTLECOIN_SERIALIZABLE_H

#include <crypto.h>

namespace BaseTypes
{
    struct IStorable : ISerializable
    {
        virtual crypto_hash_t hash() const = 0;

        virtual uint64_t type() const = 0;
    };
} // namespace BaseTypes

#endif
