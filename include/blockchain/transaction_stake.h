// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_TRANSACTION_STAKE_H
#define TURTLECOIN_TRANSACTION_STAKE_H

#include "base_types.h"

namespace Types::Blockchain
{
    struct committed_stake_transaction_t :
        BaseTypes::TransactionPrefix,
        BaseTypes::TransactionUserBody,
        BaseTypes::StakeTransactionData,
        BaseTypes::CommittedTransactionSuffix,
        virtual BaseTypes::IStorable
    {
        committed_stake_transaction_t()
        {
            l_type = BaseTypes::TransactionType::STAKE;
        }

        committed_stake_transaction_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        committed_stake_transaction_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        committed_stake_transaction_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        committed_stake_transaction_t(const std::string &hex)
        {
            deserializer_t reader(hex);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(committed_stake_transaction_t, fromJSON)

        void deserialize(deserializer_t &reader) override
        {
            deserialize_prefix(reader);

            deserialize_body(reader);

            deserialize_data(reader);

            deserialize_suffix(reader);
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            prefix_fromJSON(j);

            body_fromJSON(j);

            data_fromJSON(j);

            suffix_fromJSON(j);
        }

        [[nodiscard]] crypto_hash_t digest() const
        {
            const auto data = serialize_digest();

            return Crypto::Hashing::sha3(data.data(), data.size());
        }

        [[nodiscard]] crypto_hash_t hash() const override
        {
            serializer_t writer;

            writer.key(digest());

            /**
             * To make sure that both an uncommitted and committed transaction have the same
             * hash, we incorporate the pruning hash into the hash of the uncommitted
             * transaction here to make sure that we get the same result
             */

            pruning_hash.serialize(writer);

            writer.key(pruning_hash);

            return Crypto::Hashing::sha3(writer.data(), writer.size());
        }

        void serialize(serializer_t &writer) const override
        {
            serialize_prefix(writer);

            serialize_body(writer);

            serialize_data(writer);

            serialize_suffix(writer);
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const override
        {
            serializer_t writer;

            serialize(writer);

            return writer.vector();
        }

        [[nodiscard]] std::vector<uint8_t> serialize_digest() const
        {
            serializer_t writer;

            serialize_prefix(writer);

            serialize_body(writer);

            serialize_data(writer);

            return writer.vector();
        }

        [[nodiscard]] size_t size() const override
        {
            const auto bytes = serialize();

            return bytes.size();
        }

        JSON_TO_FUNC(toJSON) override
        {
            writer.StartObject();
            {
                prefix_toJSON(writer);

                body_toJSON(writer);

                data_toJSON(writer);

                suffix_toJSON(writer);
            }
            writer.EndObject();
        }

        [[nodiscard]] std::string to_string() const override
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }
    };

    struct uncommitted_stake_transaction_t :
        BaseTypes::TransactionPrefix,
        BaseTypes::TransactionUserBody,
        BaseTypes::StakeTransactionData,
        BaseTypes::UncommittedTransactionSuffix,
        virtual BaseTypes::IStorable
    {
        uncommitted_stake_transaction_t()
        {
            l_type = BaseTypes::TransactionType::STAKE;
        }

        uncommitted_stake_transaction_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        uncommitted_stake_transaction_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        uncommitted_stake_transaction_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        uncommitted_stake_transaction_t(const std::string &hex)
        {
            deserializer_t reader(hex);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(uncommitted_stake_transaction_t, fromJSON)

        void deserialize(deserializer_t &reader) override
        {
            deserialize_prefix(reader);

            deserialize_body(reader);

            deserialize_data(reader);

            deserialize_suffix(reader);
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            prefix_fromJSON(j);

            body_fromJSON(j);

            data_fromJSON(j);

            suffix_fromJSON(j);
        }

        [[nodiscard]] crypto_hash_t digest() const
        {
            const auto data = serialize_digest();

            return Crypto::Hashing::sha3(data.data(), data.size());
        }

        [[nodiscard]] size_t digest_size() const
        {
            const auto data = serialize_digest();

            return data.size() + sizeof(crypto_hash_t);
        }

        [[nodiscard]] crypto_hash_t hash() const override
        {
            serializer_t writer;

            writer.key(digest());

            /**
             * To make sure that both an uncommitted and committed transaction have the same
             * hash, we incorporate the pruning hash into the hash of the uncommitted
             * transaction here to make sure that we get the same result
             */

            const auto prune_hash = pruning_hash();

            writer.key(prune_hash);

            return Crypto::Hashing::sha3(writer.data(), writer.size());
        }

        [[nodiscard]] bool mine(const uint8_t zeros = 0)
        {
            auto hash = pow_hash();

            if (hash.leading_zeros() >= zeros)
            {
                return true;
            }

            nonce = 0;

            while (hash.leading_zeros() < zeros && nonce != UINT64_MAX)
            {
                nonce++;

                hash = pow_hash();
            }

            return hash.leading_zeros() >= zeros;
        }

        [[nodiscard]] crypto_hash_t pow_hash() const
        {
            serializer_t writer;

            writer.key(digest());

            writer.key(range_proof.hash());

            const auto data = Crypto::Hashing::sha3(writer.data(), writer.size());

            return Crypto::Hashing::argon2id(
                data,
                Configuration::Transaction::ProofOfWork::ITERATIONS,
                Configuration::Transaction::ProofOfWork::MEMORY,
                Configuration::Transaction::ProofOfWork::THREADS);
        }

        [[nodiscard]] bool pow_verify(const uint8_t zeros = 0) const
        {
            return pow_hash().leading_zeros() >= zeros;
        }

        [[nodiscard]] crypto_hash_t pruning_hash() const
        {
            return suffix_hash();
        }

        void serialize(serializer_t &writer) const override
        {
            serialize_prefix(writer);

            serialize_body(writer);

            serialize_data(writer);

            serialize_suffix(writer);
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const override
        {
            serializer_t writer;

            serialize(writer);

            return writer.vector();
        }

        [[nodiscard]] std::vector<uint8_t> serialize_digest() const
        {
            serializer_t writer;

            serialize_prefix(writer);

            serialize_body(writer);

            serialize_data(writer);

            return writer.vector();
        }

        [[nodiscard]] size_t size() const override
        {
            const auto bytes = serialize();

            return bytes.size();
        }

        [[nodiscard]] committed_stake_transaction_t to_committed() const
        {
            committed_stake_transaction_t tx;

            tx.version = version;

            tx.unlock_block = unlock_block;

            tx.tx_public_key = tx_public_key;

            tx.nonce = nonce;

            tx.fee = fee;

            tx.key_images = key_images;

            tx.outputs = outputs;

            tx.stake_amount = stake_amount;

            tx.candidate_public_key = candidate_public_key;

            tx.staker_public_view_key = staker_public_view_key;

            tx.staker_public_spend_key = staker_public_spend_key;

            tx.pruning_hash = pruning_hash();

            return tx;
        }

        JSON_TO_FUNC(toJSON) override
        {
            writer.StartObject();
            {
                prefix_toJSON(writer);

                body_toJSON(writer);

                data_toJSON(writer);

                suffix_toJSON(writer);
            }
            writer.EndObject();
        }

        [[nodiscard]] std::string to_string() const override
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }
    };
} // namespace Types::Blockchain

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Blockchain::committed_stake_transaction_t &value)
    {
        os << "Committed Stake Transaction [" << value.size() << " bytes]" << std::endl
           << "\tHash: " << value.hash() << std::endl
           << "\tDigest: " << value.digest() << std::endl
           << "\tPruning Hash: " << value.pruning_hash << std::endl
           << "\tVersion: " << value.version << std::endl
           << "\tUnlock Block: " << value.unlock_block << std::endl
           << "\tTx Public Key: " << value.tx_public_key << std::endl
           << "\tNonce: " << value.nonce << std::endl
           << "\tFee: " << value.fee << std::endl
           << "\tInput Key Images:" << std::endl;

        for (const auto &key_image : value.key_images)
        {
            os << "\t\t" << key_image << std::endl;
        }

        os << std::endl << "\tOutputs:" << std::endl;

        for (const auto &output : value.outputs)
        {
            os << output << std::endl;
        }

        os << "\tStake Amount: " << value.stake_amount << std::endl
           << "\tCandidate Public Key: " << value.candidate_public_key << std::endl
           << "\tStaker Public View Key: " << value.staker_public_view_key << std::endl
           << "\tStaker Public Spend Key: " << value.staker_public_spend_key << std::endl;

        return os;
    }

    inline ostream &operator<<(ostream &os, const Types::Blockchain::uncommitted_stake_transaction_t &value)
    {
        os << "Uncommitted Stake Transaction [" << value.size() << " bytes]" << std::endl
           << "\tCommitted Size: " << value.digest_size() << " bytes" << std::endl
           << "\tHash: " << value.hash() << std::endl
           << "\tDigest: " << value.digest() << std::endl
           << "\tPruning Hash: " << value.pruning_hash() << std::endl
           << "\tVersion: " << value.version << std::endl
           << "\tUnlock Block: " << value.unlock_block << std::endl
           << "\tTx Public Key: " << value.tx_public_key << std::endl
           << "\tNonce: " << value.nonce << std::endl
           << "\tFee: " << value.fee << std::endl;

        os << "\tInput Key Images:" << std::endl;

        for (const auto &key_image : value.key_images)
        {
            os << "\t\t" << key_image << std::endl;
        }

        os << std::endl << "\tOutputs:" << std::endl;

        for (const auto &output : value.outputs)
        {
            os << output << std::endl;
        }

        os << "\tStake Amount: " << value.stake_amount << std::endl
           << "\tCandidate Public Key: " << value.candidate_public_key << std::endl
           << "\tStaker Public View Key: " << value.staker_public_view_key << std::endl
           << "\tStaker Public Spend Key: " << value.staker_public_spend_key << std::endl
           << std::endl;

        os << "\tInput Offsets:" << std::endl;
        for (size_t i = 0; i < value.offsets.size(); ++i)
        {
            if (i == 0)
                os << "\t\t";
            else if (i % 8 == 0)
                os << std::endl << "\t\t";

            os << value.offsets[i] << ", ";
        }
        os << std::endl;

        for (const auto &signature : value.signatures)
        {
            os << signature << std::endl;
        }

        os << value.range_proof << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_TRANSACTION_STAKE_H
