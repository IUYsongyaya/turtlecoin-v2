// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_BLOCKCHAIN_BASE_TYPES_H
#define TURTLECOIN_BLOCKCHAIN_BASE_TYPES_H

#include "config.h"

#include <crypto.h>
#include <serializable.h>

namespace BaseTypes
{
    enum TransactionType
    {
        GENESIS,
        STAKER_REWARD,
        NORMAL,
        STAKE,
        RECALL_STAKE,
        STAKE_REFUND
    };

    struct TransactionHeader : virtual BaseTypes::IStorable
    {
        TransactionHeader() {}

        TransactionHeader(deserializer_t &reader)
        {
            deserialize_header(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(TransactionHeader, header_fromJSON)

        void deserialize_header(deserializer_t &reader)
        {
            l_type = reader.varint<uint64_t>();

            version = reader.varint<uint64_t>();
        }

        void serialize_header(serializer_t &writer) const
        {
            writer.varint(l_type);

            writer.varint(version);
        }

        void header_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.Key("type");
            writer.Uint64(l_type);

            U64_TO_JSON(version);
        }

        JSON_FROM_FUNC(header_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("type");

            l_type = get_json_uint64_t(j, "type");

            LOAD_U64_FROM_JSON(version);
        }

        uint64_t type() const override
        {
            return l_type;
        }

        uint64_t version = 0;

      protected:
        uint64_t l_type = 0;
    };

    struct TransactionPrefix : TransactionHeader
    {
        TransactionPrefix() {}

        TransactionPrefix(deserializer_t &reader)
        {
            deserialize_prefix(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(TransactionPrefix, prefix_fromJSON)

        void deserialize_prefix(deserializer_t &reader)
        {
            deserialize_header(reader);

            unlock_block = reader.varint<uint64_t>();

            tx_public_key = reader.key<crypto_public_key_t>();
        }

        void serialize_prefix(serializer_t &writer) const
        {
            serialize_header(writer);

            writer.varint(unlock_block);

            writer.key(tx_public_key);
        }

        JSON_TO_FUNC(prefix_toJSON)
        {
            header_toJSON(writer);

            U64_TO_JSON(unlock_block);

            KEY_TO_JSON(tx_public_key);
        }

        JSON_FROM_FUNC(prefix_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            header_fromJSON(j);

            LOAD_U64_FROM_JSON(unlock_block);

            LOAD_KEY_FROM_JSON(tx_public_key);
        }

        uint64_t unlock_block = 0;
        crypto_public_key_t tx_public_key;
    };

    struct StakerOutput
    {
        StakerOutput() {}

        StakerOutput(const crypto_hash_t &staker_id, uint64_t amount): staker_id(staker_id), amount(amount) {}

        StakerOutput(deserializer_t &reader)
        {
            deserialize_output(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(StakerOutput, output_fromJSON)

        void deserialize_output(deserializer_t &reader)
        {
            staker_id = reader.key<crypto_hash_t>();

            amount = reader.varint<uint64_t>();
        }

        void serialize_output(serializer_t &writer) const
        {
            writer.key(staker_id);

            writer.varint(amount);
        }

        JSON_TO_FUNC(output_toJSON)
        {
            writer.StartObject();
            {
                KEY_TO_JSON(staker_id);

                U64_TO_JSON(amount);
            }
            writer.EndObject();
        }

        JSON_FROM_FUNC(output_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            LOAD_KEY_FROM_JSON(staker_id);

            LOAD_U64_FROM_JSON(amount);
        }

        crypto_hash_t staker_id;
        uint64_t amount = 0;
    };

    struct TransactionOutput
    {
        TransactionOutput() {}

        TransactionOutput(
            const crypto_public_key_t &public_ephemeral,
            const uint64_t &amount,
            const crypto_pedersen_commitment_t &commitment):
            public_ephemeral(public_ephemeral), amount(amount), commitment(commitment)
        {
        }

        TransactionOutput(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize_output(reader);
        }

        TransactionOutput(deserializer_t &reader)
        {
            deserialize_output(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(TransactionOutput, output_fromJSON)

        void deserialize_output(deserializer_t &reader)
        {
            public_ephemeral = reader.key<crypto_public_key_t>();

            amount = reader.varint<uint64_t>();

            commitment = reader.key<crypto_pedersen_commitment_t>();
        }

        void serialize_output(serializer_t &writer) const
        {
            writer.key(public_ephemeral);

            writer.varint(amount);

            writer.key(commitment);
        }

        std::vector<uint8_t> serialize_output() const
        {
            serializer_t writer;

            serialize_output(writer);

            return writer.vector();
        }

        JSON_TO_FUNC(output_toJSON)
        {
            writer.StartObject();
            {
                KEY_TO_JSON(public_ephemeral);

                U64_TO_JSON(amount);

                KEY_TO_JSON(commitment);
            }
            writer.EndObject();
        }

        JSON_FROM_FUNC(output_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            LOAD_KEY_FROM_JSON(public_ephemeral);

            LOAD_U64_FROM_JSON(amount);

            LOAD_KEY_FROM_JSON(commitment);
        }

        crypto_public_key_t public_ephemeral;
        uint64_t amount = 0;
        crypto_pedersen_commitment_t commitment;
    };

    struct TransactionUserBody
    {
        TransactionUserBody() {}

        void deserialize_body(deserializer_t &reader)
        {
            nonce = reader.varint<uint64_t>();

            fee = reader.varint<uint64_t>();

            key_images = reader.keyV<crypto_key_image_t>();

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

        void serialize_body(serializer_t &writer) const
        {
            writer.varint(nonce);

            writer.varint(fee);

            writer.key(key_images);

            writer.varint(outputs.size());

            for (const auto &output : outputs)
            {
                output.serialize_output(writer);
            }
        }

        JSON_TO_FUNC(body_toJSON)
        {
            U64_TO_JSON(nonce);

            U64_TO_JSON(fee);

            KEYV_TO_JSON(key_images);

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

        JSON_FROM_FUNC(body_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            LOAD_U64_FROM_JSON(nonce);

            LOAD_U64_FROM_JSON(fee);

            LOAD_KEYV_FROM_JSON(key_images);

            JSON_MEMBER_OR_THROW("outputs");

            outputs.clear();

            for (const auto &elem : get_json_array(j, "outputs"))
            {
                outputs.emplace_back(elem);
            }
        }

        uint64_t nonce = 0;
        uint64_t fee = 0;
        std::vector<crypto_key_image_t> key_images;
        std::vector<TransactionOutput> outputs;
    };

    struct UncommittedTransactionSuffix
    {
        UncommittedTransactionSuffix() {}

        [[nodiscard]] crypto_hash_t suffix_hash() const
        {
            serializer_t writer;

            serialize_suffix(writer);

            return Crypto::Hashing::sha3(writer.data(), writer.size());
        }

        void deserialize_suffix(deserializer_t &reader)
        {
            offsets = reader.varintV<uint64_t>();

            signatures.clear();

            {
                const auto count = reader.varint<uint64_t>();

                for (size_t i = 0; i < count; ++i)
                {
                    crypto_clsag_signature_t signature;

                    signature.deserialize(reader);

                    signatures.push_back(signature);
                }
            }

            range_proof.deserialize(reader);
        }

        void serialize_suffix(serializer_t &writer) const
        {
            writer.varint(offsets);

            writer.varint(signatures.size());

            for (const auto &signature : signatures)
            {
                signature.serialize(writer);
            }

            range_proof.serialize(writer);
        }

        JSON_TO_FUNC(suffix_toJSON)
        {
            writer.Key("offsets");
            writer.StartArray();
            {
                for (const auto &offset : offsets)
                {
                    writer.Uint64(offset);
                }
            }
            writer.EndArray();

            KEYV_TO_JSON(signatures);

            writer.Key("range_proof");
            range_proof.toJSON(writer);
        }

        JSON_FROM_FUNC(suffix_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("offsets");

            offsets.clear();

            for (const auto &elem : get_json_array(j, "offsets"))
            {
                const auto offset = get_json_uint64_t(elem);

                offsets.emplace_back(offset);
            }

            LOAD_KEYV_FROM_JSON(signatures);

            JSON_MEMBER_OR_THROW("range_proof");

            range_proof = crypto_bulletproof_plus_t(j, "range_proof");
        }

        std::vector<uint64_t> offsets;
        std::vector<crypto_clsag_signature_t> signatures;
        crypto_bulletproof_plus_t range_proof;
    };

    struct CommittedTransactionSuffix
    {
        CommittedTransactionSuffix() {}

        [[nodiscard]] crypto_hash_t suffix_hash() const
        {
            return pruning_hash;
        }

        void deserialize_suffix(deserializer_t &reader)
        {
            pruning_hash = reader.key<crypto_hash_t>();
        }

        void serialize_suffix(serializer_t &writer) const
        {
            writer.key(pruning_hash);
        }

        JSON_TO_FUNC(suffix_toJSON)
        {
            KEY_TO_JSON(pruning_hash);
        }

        JSON_FROM_FUNC(suffix_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            LOAD_KEY_FROM_JSON(pruning_hash);
        }

        crypto_hash_t pruning_hash;
    };

    struct NormalTransactionData
    {
        NormalTransactionData() {}

        void deserialize_data(deserializer_t &reader)
        {
            // data
            {
                const auto count = reader.varint<uint64_t>();

                tx_extra = reader.bytes(count);
            }
        }

        void serialize_data(serializer_t &writer) const
        {
            writer.varint(tx_extra.size());

            writer.bytes(tx_extra);
        }

        JSON_TO_FUNC(data_toJSON)
        {
            writer.Key("tx_extra");

            writer.Key(Crypto::StringTools::to_hex(tx_extra.data(), tx_extra.size()));
        }

        JSON_FROM_FUNC(data_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("tx_extra");

            const auto extra = get_json_string(j, "tx_extra");

            tx_extra = Crypto::StringTools::from_hex(extra);
        }

        std::vector<uint8_t> tx_extra;
    };

    struct StakeTransactionData
    {
        StakeTransactionData() {}

        void deserialize_data(deserializer_t &reader)
        {
            stake_amount = reader.varint<uint64_t>();

            candidate_public_key = reader.key<crypto_public_key_t>();

            staker_public_view_key = reader.key<crypto_public_key_t>();

            staker_public_spend_key = reader.key<crypto_public_key_t>();
        }

        void serialize_data(serializer_t &writer) const
        {
            writer.varint(stake_amount);

            writer.key(candidate_public_key);

            writer.key(staker_public_view_key);

            writer.key(staker_public_spend_key);
        }

        JSON_TO_FUNC(data_toJSON)
        {
            U64_TO_JSON(stake_amount);

            KEY_TO_JSON(candidate_public_key);

            KEY_TO_JSON(staker_public_view_key);

            KEY_TO_JSON(staker_public_spend_key);
        }

        JSON_FROM_FUNC(data_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            LOAD_U64_FROM_JSON(stake_amount);

            LOAD_KEY_FROM_JSON(candidate_public_key);

            LOAD_KEY_FROM_JSON(staker_public_view_key);

            LOAD_KEY_FROM_JSON(staker_public_spend_key);
        }

        uint64_t stake_amount = 0;
        crypto_public_key_t candidate_public_key, staker_public_view_key, staker_public_spend_key;
    };

    struct RecallStakeTransactionData
    {
        RecallStakeTransactionData() {}

        void deserialize_data(deserializer_t &reader)
        {
            stake_amount = reader.varint<uint64_t>();

            candidate_public_key = reader.key<crypto_public_key_t>();

            staker_id = reader.key<crypto_hash_t>();

            view_signature = reader.key<crypto_signature_t>();

            spend_signature = reader.key<crypto_signature_t>();
        }

        void serialize_data(serializer_t &writer) const
        {
            writer.varint(stake_amount);

            writer.key(candidate_public_key);

            writer.key(staker_id);

            writer.key(view_signature);

            writer.key(spend_signature);
        }

        JSON_TO_FUNC(data_toJSON)
        {
            U64_TO_JSON(stake_amount);

            KEY_TO_JSON(candidate_public_key);

            KEY_TO_JSON(staker_id);

            KEY_TO_JSON(view_signature);

            KEY_TO_JSON(spend_signature);
        }

        JSON_FROM_FUNC(data_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            LOAD_U64_FROM_JSON(stake_amount);

            LOAD_KEY_FROM_JSON(candidate_public_key);

            LOAD_KEY_FROM_JSON(staker_id);

            LOAD_KEY_FROM_JSON(view_signature);

            LOAD_KEY_FROM_JSON(spend_signature);
        }

        uint64_t stake_amount = 0;
        crypto_public_key_t candidate_public_key;
        crypto_hash_t staker_id;
        crypto_signature_t view_signature, spend_signature;
    };
} // namespace BaseTypes

constexpr std::string_view canary = "TurtleCoin";

constexpr std::string_view canaryObfusticated = "\u0054\u0075\u0072\u0074\u006c\u0065\u0043\u006f\u0069\u006e";

/* Compare the canary with T u r t l e C o i n. Done as unicode to not be caught
   by find and replace. If find and replace occurred, the canary will no longer
   match. Then we can warn them that they have probably replaced license headers,
   and halt compilation till it's fixed. */
static_assert(
    canary == canaryObfusticated,
    "\n\n\n\n\u0057\u006F\u0061\u0068\u0021\u0020\u0057\u0061\u0069\u0074\u0020\u0061\u0020\u006D\u0069\u006E\u0075"
    "\u0074\u0065\u002C\u0020\u0068\u006F\u006C\u0064\u0020\u0075\u0070\u002C\u0020\u0062\u0061\u0063\u006B\u0020\u0075"
    "\u0070\u0020\u0074\u0068\u0061\u0074\u0020\u0054\u0075\u0072\u0074\u006C\u0065\u002E\u002E\u002E\u000A\u000A\u0049"
    "\u0074\u0020\u0073\u0065\u0065\u006D\u0073\u0020\u0074\u0068\u0061\u0074\u0020\u0079\u006F\u0075\u0020\u0068\u0061"
    "\u0076\u0065\u0020\u0066\u006F\u0072\u006B\u0065\u0064\u0020\u0074\u0068\u0069\u0073\u0020\u0070\u0072\u006F\u006A"
    "\u0065\u0063\u0074\u0020\u0061\u006E\u0064\u0020\u0075\u0073\u0065\u0064\u0020\u0061\u0020\u0073\u0069\u006D\u0070"
    "\u006C\u0065\u0020\u0072\u0065\u0070\u006C\u0061\u0063\u0065\u0020\u0061\u006C\u006C\u0020\u0074\u006F\u0020\u006D"
    "\u0061\u006B\u0065\u0020\u0074\u0068\u0065\u0020\u0070\u0072\u006F\u006A\u0065\u0063\u0074\u0020\u0079\u006F\u0075"
    "\u0072\u0020\u006F\u0077\u006E\u002E\u000A\u000A\u0059\u006F\u0075\u0020\u0061\u0072\u0065\u0020\u0075\u006E\u0064"
    "\u006F\u0075\u0062\u0074\u0065\u0064\u006C\u0079\u0020\u0069\u006E\u0020\u0076\u0069\u006F\u006C\u0061\u0074\u0069"
    "\u006F\u006E\u0020\u006F\u0066\u0020\u0074\u0068\u0065\u0020\u004C\u0049\u0043\u0045\u004E\u0053\u0045\u0020\u0074"
    "\u0068\u0069\u0073\u0020\u0073\u006F\u0066\u0074\u0077\u0061\u0072\u0065\u0020\u0069\u0073\u0020\u0072\u0065\u006C"
    "\u0065\u0061\u0073\u0065\u0064\u0020\u0075\u006E\u0064\u0065\u0072\u0020\u0074\u0068\u0061\u0074\u0020\u0065\u0078"
    "\u0070\u006C\u0069\u0063\u0069\u0074\u006C\u0079\u0020\u0070\u0072\u006F\u0068\u0069\u0062\u0069\u0074\u0073\u0020"
    "\u0079\u006F\u0075\u0020\u0066\u0072\u006F\u006D\u0020\u0064\u006F\u0069\u006E\u0067\u0020\u0074\u0068\u0069\u0073"
    "\u002E\u000A\u000A\u0049\u0066\u0020\u0079\u006F\u0075\u0020\u006E\u0065\u0065\u0064\u0020\u0068\u0065\u006C\u0070"
    "\u0020\u006D\u0061\u006B\u0069\u006E\u0067\u0020\u0073\u0075\u0072\u0065\u0020\u0074\u0068\u0061\u0074\u0020\u0077"
    "\u0068\u0061\u0074\u0020\u0079\u006F\u0075\u0020\u0061\u0072\u0065\u0020\u0064\u006F\u0069\u006E\u0067\u0020\u0069"
    "\u0073\u0020\u0070\u0065\u0072\u006D\u0069\u0074\u0074\u0065\u0064\u0020\u0075\u006E\u0064\u0065\u0072\u0020\u0074"
    "\u0068\u0065\u0020\u004C\u0049\u0043\u0045\u004E\u0053\u0045\u002C\u0020\u0073\u0077\u0069\u006E\u0067\u0020\u006F"
    "\u006E\u0020\u0062\u0079\u0020\u0068\u0074\u0074\u0070\u003A\u002F\u002F\u0063\u0068\u0061\u0074\u002E\u0074\u0075"
    "\u0072\u0074\u006C\u0065\u0063\u006F\u0069\u006E\u002E\u006C\u006F\u006C\u0020\u0061\u006E\u0064\u0020\u0077\u0065"
    "\u0020\u0077\u0069\u006C\u006C\u0020\u0062\u0065\u0020\u0068\u0061\u0070\u0070\u0079\u0020\u0074\u006F\u0020\u0067"
    "\u0075\u0069\u0064\u0065\u0020\u0079\u006F\u0075\u002E\n\n\n\n");

namespace Types::Blockchain
{
    typedef BaseTypes::TransactionOutput transaction_output_t;

    typedef BaseTypes::StakerOutput staker_output_t;
} // namespace Types::Blockchain

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Blockchain::transaction_output_t &value)
    {
        os << "\tTransaction Output" << std::endl
           << "\t\tPublic Ephemeral: " << value.public_ephemeral << std::endl
           << "\t\tAmount: " << value.amount << std::endl
           << "\t\tCommitment: " << value.commitment << std::endl;

        return os;
    }

    inline ostream &operator<<(ostream &os, const Types::Blockchain::staker_output_t &value)
    {
        os << "\tStaker Output" << std::endl
           << "\t\tStaker ID: " << value.staker_id << std::endl
           << "\t\tAmount: " << value.amount << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_BLOCKCHAIN_BASE_TYPES_H
