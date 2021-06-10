// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_TRANSACTION_STAKER_REWARD_H
#define TURTLECOIN_TRANSACTION_STAKER_REWARD_H

#include "base_types.h"

namespace Types::Blockchain
{
    struct staker_reward_transaction_t : BaseTypes::TransactionHeader, virtual BaseTypes::IStorable
    {
        staker_reward_transaction_t()
        {
            l_type = BaseTypes::TransactionType::STAKER_REWARD;
        }

        staker_reward_transaction_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        staker_reward_transaction_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        staker_reward_transaction_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        staker_reward_transaction_t(const std::string &hex)
        {
            deserializer_t reader(hex);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(staker_reward_transaction_t, fromJSON)

        void deserialize(deserializer_t &reader)
        {
            deserialize_header(reader);

            // Staker Outputs
            {
                const auto count = reader.varint<uint64_t>();

                staker_outputs.clear();

                for (size_t i = 0; i < count; ++i)
                {
                    staker_outputs.emplace_back(reader);
                }
            }
        }

        JSON_FROM_FUNC(fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            header_fromJSON(j);

            JSON_MEMBER_OR_THROW("staker_outputs");

            staker_outputs.clear();

            for (const auto &elem : get_json_array(j, "staker_outputs"))
            {
                staker_outputs.emplace_back(elem);
            }
        }

        [[nodiscard]] crypto_hash_t hash() const
        {
            const auto bytes = serialize();

            return Crypto::Hashing::sha3(bytes.data(), bytes.size());
        }

        void serialize(serializer_t &writer) const
        {
            serialize_header(writer);

            writer.varint(staker_outputs.size());

            for (const auto &staker_output : staker_outputs)
            {
                staker_output.serialize_output(writer);
            }
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const
        {
            serializer_t writer;

            serialize(writer);

            return writer.vector();
        }

        [[nodiscard]] size_t size() const
        {
            const auto bytes = serialize();

            return bytes.size();
        }

        JSON_TO_FUNC(toJSON) override
        {
            writer.StartObject();
            {
                header_toJSON(writer);

                writer.Key("staker_outputs");
                writer.StartArray();
                {
                    for (const auto &staker_output : staker_outputs)
                    {
                        staker_output.output_toJSON(writer);
                    }
                }
                writer.EndArray();
            }
            writer.EndObject();
        }

        [[nodiscard]] std::string to_string() const
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }

        std::vector<staker_output_t> staker_outputs;
    };
} // namespace Types::Blockchain

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Blockchain::staker_reward_transaction_t &value)
    {
        os << "Staker Reward Transaction [" << value.size() << " bytes]" << std::endl
           << "\tHash: " << value.hash() << std::endl
           << "\tVersion: " << value.version << std::endl
           << "\tStaker Outputs:" << std::endl;

        for (const auto &output : value.staker_outputs)
        {
            os << output << std::endl;
        }

        return os;
    }
} // namespace std

#endif
