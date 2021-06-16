// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_STAKER_H
#define TURTLECOIN_STAKER_H

#include <crypto.h>

namespace Types::Staking
{
    struct staker_t : virtual BaseTypes::IStorable
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

        JSON_OBJECT_CONSTRUCTORS(staker_t, fromJSON);

        void deserialize(deserializer_t &reader) override
        {
            record_version = reader.varint<uint64_t>();

            public_view_key = reader.key<crypto_public_key_t>();

            public_spend_key = reader.key<crypto_public_key_t>();
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            LOAD_U64_FROM_JSON(record_version);

            LOAD_KEY_FROM_JSON(public_view_key);

            LOAD_KEY_FROM_JSON(public_spend_key);
        }

        /**
         * Calculates the hash of the structure
         * @return
         */
        [[nodiscard]] crypto_hash_t hash() const override
        {
            const auto data = serialize();

            return Crypto::Hashing::sha3(data.data(), data.size());
        }

        crypto_hash_t id() const
        {
            const auto bytes = serialize();

            return Crypto::Hashing::sha3(bytes.data(), bytes.size());
        }

        void serialize(serializer_t &writer) const override
        {
            writer.varint(record_version);

            public_view_key.serialize(writer);

            public_spend_key.serialize(writer);
        }

        std::vector<uint8_t> serialize() const override
        {
            serializer_t writer;

            serialize(writer);

            return writer.vector();
        }

        size_t size() const override
        {
            return serialize().size();
        }

        JSON_TO_FUNC(toJSON) override
        {
            writer.StartObject();
            {
                U64_TO_JSON(record_version);

                KEY_TO_JSON(public_view_key);

                KEY_TO_JSON(public_spend_key);
            }
            writer.EndObject();
        }

        [[nodiscard]] std::string to_string() const override
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }

        [[nodiscard]] uint64_t type() const override
        {
            return 0;
        }

        uint64_t version() const
        {
            return record_version;
        }

        crypto_public_key_t public_view_key, public_spend_key;

      private:
        /**
         * This allows us to signify updates to the record schema in the future
         */
        uint64_t record_version = Configuration::Staking::STAKER_RECORD_VERSION;
    };
} // namespace Types::Staking

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Staking::staker_t &value)
    {
        os << "Staker [v" << value.version() << "]" << std::endl
           << "\tID: " << value.id() << std::endl
           << "\tPublic View Key: " << value.public_view_key << std::endl
           << "\tPublic Spend Key: " << value.public_spend_key << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_STAKER_H
