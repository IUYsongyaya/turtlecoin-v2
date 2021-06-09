// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_TRANSACTION_GENESIS_H
#define TURTLECOIN_TRANSACTION_GENESIS_H

#include "base_types.h"

namespace Types::Blockchain
{
    struct genesis_transaction_t : BaseTypes::TransactionPrefix, virtual BaseTypes::IBlockchainSerializable
    {
        genesis_transaction_t()
        {
            l_type = BaseTypes::TransactionType::GENESIS;
        }

        genesis_transaction_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        genesis_transaction_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        genesis_transaction_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        genesis_transaction_t(const std::string &hex)
        {
            deserializer_t reader(hex);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(genesis_transaction_t, fromJSON)

        void deserialize(deserializer_t &reader) override
        {
            deserialize_prefix(reader);

            tx_secret_key = reader.key<crypto_secret_key_t>();

            // outputs
            {
                const auto count = reader.varint<uint64_t>();

                outputs.clear();

                for (size_t i = 0; i < count; ++i)
                {
                    outputs.emplace_back(reader);
                }
            }
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            prefix_fromJSON(j);

            JSON_MEMBER_OR_THROW("tx_secret_key");

            tx_secret_key = get_json_string(j, "tx_secret_key");

            JSON_MEMBER_OR_THROW("outputs");

            outputs.clear();

            for (const auto &elem : get_json_array(j, "outputs"))
            {
                outputs.emplace_back(elem);
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

            writer.varint(outputs.size());

            for (const auto &output : outputs)
            {
                output.serialize_output(writer);
            }
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

        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const override
        {
            writer.StartObject();
            {
                prefix_toJSON(writer);

                writer.Key("tx_secret_key");
                tx_secret_key.toJSON(writer);

                writer.Key("outputs");
                writer.StartArray();
                {
                    for (const auto &output : outputs)
                    {
                        output.output_toJSON(writer);
                    }
                }
                writer.EndArray();
            }
            writer.EndObject();
        }

        [[nodiscard]] std::string to_string() const override
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }

        crypto_secret_key_t tx_secret_key;
        std::vector<transaction_output_t> outputs;
    };
} // namespace Types::Blockchain

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Blockchain::genesis_transaction_t &value)
    {
        os << "Genesis Transaction [" << value.size() << " bytes]" << std::endl
           << "\tHash: " << value.hash() << std::endl
           << "\tVersion: " << value.version << std::endl
           << "\tUnlock Block: " << value.unlock_block << std::endl
           << "\tTx Public Key: " << value.tx_public_key << std::endl
           << "\tTx Secret Key: " << value.tx_secret_key << std::endl
           << "\tOutputs:" << std::endl;

        for (const auto &output : value.outputs)
        {
            os << output << std::endl;
        }

        return os;
    }
} // namespace std

#endif // TURTLECOIN_TRANSACTION_GENESIS_H
