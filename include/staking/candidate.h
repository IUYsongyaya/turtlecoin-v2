// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_CANDIDATE_H
#define TURTLECOIN_CANDIDATE_H

#include <config.h>
#include <crypto.h>

namespace TurtleCoin::Types::Staking
{
    struct candidate_node_t
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

        void serialize(serializer_t &writer) const
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

        uint64_t version() const
        {
            return record_version;
        }

        crypto_public_key_t public_signing_key, public_view_key, public_spend_key;

        crypto_hash_t staking_hash;

        uint64_t initial_stake = 0, blocks_produced = 0, blocks_validated = 0, block_production_assigned = 0,
                 block_validation_assigned = 0;

      private:
        void deserialize(deserializer_t &reader)
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

        /**
         * This allows us to signify updates to the record schema in the future
         */
        uint64_t record_version = Configuration::Staking::CANDIDATE_RECORD_VERSION;
    };
} // namespace TurtleCoin::Types::Staking

namespace std
{
    inline ostream &operator<<(ostream &os, const TurtleCoin::Types::Staking::candidate_node_t &value)
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
