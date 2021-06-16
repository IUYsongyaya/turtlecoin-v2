// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_CANDIDATE_H
#define TURTLECOIN_CANDIDATE_H

#include <config.h>
#include <crypto.h>
#include <serializable.h>

namespace Types::Staking
{
    struct candidate_node_t : virtual BaseTypes::IStorable
    {
        candidate_node_t() {};

        candidate_node_t(
            const crypto_public_key_t &public_signing_key,
            const crypto_public_key_t &public_view_key,
            const crypto_public_key_t &public_spend_key,
            const crypto_hash_t &staking_hash,
            uint64_t initial_stake = 0):
            public_signing_key(public_signing_key),
            public_view_key(public_view_key),
            public_spend_key(public_spend_key),
            staking_hash(staking_hash),
            initial_stake(initial_stake)
        {
        }

        candidate_node_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        candidate_node_t(const std::vector<uint8_t> &data)
        {
            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        candidate_node_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(candidate_node_t, fromJSON);

        void deserialize(deserializer_t &reader) override
        {
            record_version = reader.varint<uint64_t>();

            public_signing_key = reader.key<crypto_public_key_t>();

            public_view_key = reader.key<crypto_public_key_t>();

            public_spend_key = reader.key<crypto_public_key_t>();

            staking_hash = reader.key<crypto_hash_t>();

            initial_stake = reader.varint<uint64_t>();

            block_production_assigned = reader.varint<uint64_t>();

            block_validation_assigned = reader.varint<uint64_t>();

            blocks_produced = reader.varint<uint64_t>();

            blocks_validated = reader.varint<uint64_t>();
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            LOAD_U64_FROM_JSON(record_version);

            LOAD_KEY_FROM_JSON(public_signing_key);

            LOAD_KEY_FROM_JSON(public_view_key);

            LOAD_KEY_FROM_JSON(public_spend_key);

            LOAD_KEY_FROM_JSON(staking_hash);

            LOAD_U64_FROM_JSON(initial_stake);

            LOAD_U64_FROM_JSON(block_production_assigned);

            LOAD_U64_FROM_JSON(block_validation_assigned);

            LOAD_U64_FROM_JSON(blocks_produced);

            LOAD_U64_FROM_JSON(blocks_validated);
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

        void serialize(serializer_t &writer) const override
        {
            writer.varint(record_version);

            public_signing_key.serialize(writer);

            public_view_key.serialize(writer);

            public_spend_key.serialize(writer);

            staking_hash.serialize(writer);

            writer.varint(initial_stake);

            writer.varint(block_production_assigned);

            writer.varint(block_validation_assigned);

            writer.varint(blocks_produced);

            writer.varint(blocks_validated);
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

                KEY_TO_JSON(public_signing_key);

                KEY_TO_JSON(public_view_key);

                KEY_TO_JSON(public_spend_key);

                KEY_TO_JSON(staking_hash);

                U64_TO_JSON(initial_stake);

                U64_TO_JSON(block_production_assigned);

                U64_TO_JSON(block_validation_assigned);

                U64_TO_JSON(blocks_produced);

                U64_TO_JSON(blocks_validated);
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

        crypto_public_key_t public_signing_key, public_view_key, public_spend_key;

        crypto_hash_t staking_hash;

        uint64_t initial_stake = 0, blocks_produced = 0, blocks_validated = 0, block_production_assigned = 0,
                 block_validation_assigned = 0;

      private:
        /**
         * This allows us to signify updates to the record schema in the future
         */
        uint64_t record_version = Configuration::Staking::CANDIDATE_RECORD_VERSION;
    };
} // namespace Types::Staking

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Staking::candidate_node_t &value)
    {
        os << "Candidate Node [v" << value.version() << "]" << std::endl
           << "\tStaking Hash: " << value.staking_hash << std::endl
           << "\tPublic Signing Key: " << value.public_signing_key << std::endl
           << "\tPublic View Key: " << value.public_view_key << std::endl
           << "\tPublic Spend Key: " << value.public_spend_key << std::endl
           << "\tInitial Stake: " << value.initial_stake << std::endl
           << "\tBlock Productions Assigned: " << value.block_production_assigned << std::endl
           << "\tBlocks Produced: " << value.blocks_produced << std::endl
           << "\tBlock Validations Assigned: " << value.block_production_assigned << std::endl
           << "\tBlocks Validated: " << value.blocks_validated << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_CANDIDATE_H
