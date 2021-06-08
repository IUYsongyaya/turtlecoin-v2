// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_STAKER_H
#define TURTLECOIN_STAKER_H

#include <crypto.h>

namespace TurtleCoin::Types::Staking
{
    struct staker_t
    {
        staker_t() {}

        staker_t(const crypto_public_key_t &public_view_key, const crypto_public_key_t &public_spend_key):
            public_view_key(public_view_key), public_spend_key(public_spend_key)
        {
        }

        staker_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        staker_t(const std::vector<uint8_t> &data)
        {
            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        staker_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        crypto_hash_t id() const
        {
            const auto bytes = serialize();

            return Crypto::Hashing::sha3(bytes.data(), bytes.size());
        }

        void serialize(serializer_t &writer) const
        {
            writer.varint(record_version);

            public_view_key.serialize(writer);

            public_spend_key.serialize(writer);
        }

        std::vector<uint8_t> serialize() const
        {
            serializer_t writer;

            serialize(writer);

            return writer.vector();
        }

        size_t size() const
        {
            return serialize().size();
        }

        [[nodiscard]] std::string to_string() const
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }

        uint64_t version() const
        {
            return record_version;
        }

        crypto_public_key_t public_view_key, public_spend_key;

      private:
        void deserialize(deserializer_t &reader)
        {
            record_version = reader.varint<uint64_t>();

            public_view_key = reader.key<crypto_public_key_t>();

            public_spend_key = reader.key<crypto_public_key_t>();
        }

        /**
         * This allows us to signify updates to the record schema in the future
         */
        uint64_t record_version = Configuration::Staking::STAKER_RECORD_VERSION;
    };
} // namespace TurtleCoin::Types::Staking

namespace std
{
    inline ostream &operator<<(ostream &os, const TurtleCoin::Types::Staking::staker_t &value)
    {
        os << "Staker [v" << value.version() << "]" << std::endl
           << "\tID: " << value.id() << std::endl
           << "\tPublic View Key: " << value.public_view_key << std::endl
           << "\tPublic Spend Key: " << value.public_spend_key << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_STAKER_H
