// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_STAKE_H
#define TURTLECOIN_STAKE_H

#include <crypto.h>

namespace Types::Staking
{
    struct stake_t
    {
        stake_t() {}

        stake_t(const crypto_hash_t &staker_id, const crypto_hash_t &stake_txn, uint64_t stake):
            staker_id(staker_id), stake_txn(stake_txn), stake(stake)
        {
        }

        stake_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        stake_t(const std::vector<uint8_t> &data)
        {
            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        stake_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        void serialize(serializer_t &writer) const
        {
            writer.varint(record_version);

            staker_id.serialize(writer);

            stake_txn.serialize(writer);

            writer.varint(stake);
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

        crypto_hash_t staker_id;
        crypto_hash_t stake_txn;
        uint64_t stake = 0;

      private:
        void deserialize(deserializer_t &reader)
        {
            record_version = reader.varint<uint64_t>();

            staker_id = reader.key<crypto_hash_t>();

            stake_txn = reader.key<crypto_hash_t>();

            stake = reader.varint<uint64_t>();
        }

        /**
         * This allows us to signify updates to the record schema in the future
         */
        uint64_t record_version = Configuration::Staking::STAKE_RECORD_VERSION;
    };
} // namespace Types::Staking

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Staking::stake_t &value)
    {
        os << "Stake [v" << value.version() << "]" << std::endl
           << "Staker ID: " << value.staker_id << std::endl
           << "Stake Txn: " << value.stake_txn << std::endl
           << "Stake Amount: " << value.stake << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_STAKE_H
