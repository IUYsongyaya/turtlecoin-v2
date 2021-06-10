// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_TRANSACTION_STAKE_REFUND_H
#define TURTLECOIN_TRANSACTION_STAKE_REFUND_H

#include "base_types.h"

namespace Types::Blockchain
{
    struct stake_refund_transaction_t :
        BaseTypes::TransactionPrefix,
        BaseTypes::TransactionOutput,
        virtual BaseTypes::IStorable
    {
        stake_refund_transaction_t()
        {
            l_type = BaseTypes::TransactionType::STAKE_REFUND;
        }

        stake_refund_transaction_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        stake_refund_transaction_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        stake_refund_transaction_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        stake_refund_transaction_t(const std::string &hex)
        {
            deserializer_t reader(hex);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(stake_refund_transaction_t, fromJSON)

        void deserialize(deserializer_t &reader) override
        {
            deserialize_prefix(reader);

            tx_secret_key = reader.key<crypto_secret_key_t>();

            recall_stake_tx = reader.key<crypto_hash_t>();

            deserialize_output(reader);
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            prefix_fromJSON(j);

            LOAD_KEY_FROM_JSON(tx_secret_key);

            LOAD_KEY_FROM_JSON(recall_stake_tx);

            JSON_MEMBER_OR_THROW("output");

            {
                const auto &elem = get_json_value(j, "output");

                output_fromJSON(elem);
            }
        }

        [[nodiscard]] crypto_hash_t hash() const override
        {
            const auto bytes = serialize();

            return Crypto::Hashing::sha3(bytes.data(), bytes.size());
        }

        void serialize(serializer_t &writer) const override
        {
            serialize_prefix(writer);

            tx_secret_key.serialize(writer);

            recall_stake_tx.serialize(writer);

            serialize_output(writer);
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const override
        {
            serializer_t writer;

            serialize(writer);

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

                KEY_TO_JSON(tx_secret_key);

                KEY_TO_JSON(recall_stake_tx);

                writer.Key("output");
                output_toJSON(writer);
            }
            writer.EndObject();
        }

        [[nodiscard]] std::string to_string() const override
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }

        crypto_secret_key_t tx_secret_key;
        crypto_hash_t recall_stake_tx;
    };
} // namespace Types::Blockchain

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Blockchain::stake_refund_transaction_t &value)
    {
        os << "Stake Refund Transaction [" << value.size() << " bytes]" << std::endl
           << "\tHash: " << value.hash() << std::endl
           << "\tVersion: " << value.version << std::endl
           << "\tUnlock Block: " << value.unlock_block << std::endl
           << "\tTx Public Key: " << value.tx_public_key << std::endl
           << "\tTx Secret key: " << value.tx_secret_key << std::endl
           << "\tRecall Stake Tx: " << value.recall_stake_tx << std::endl
           << "\tPublic Ephemeral: " << value.public_ephemeral << std::endl
           << "\tAmount: " << value.amount << std::endl
           << "\tCommitment: " << value.commitment << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_TRANSACTION_STAKE_REFUND_H
