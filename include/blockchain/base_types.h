// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_BLOCKCHAIN_BASE_TYPES_H
#define TURTLECOIN_BLOCKCHAIN_BASE_TYPES_H

#include "config.h"

#include <crypto.h>

namespace TurtleCoin::BaseTypes
{
    struct TransactionHeader
    {
        TransactionHeader() {}

        TransactionHeader(deserializer_t &reader)
        {
            deserialize_header(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(TransactionHeader, header_fromJSON)

        void deserialize_header(deserializer_t &reader)
        {
            type = reader.varint<uint64_t>();

            version = reader.varint<uint64_t>();
        }

        void serialize_header(serializer_t &writer) const
        {
            writer.varint(type);

            writer.varint(version);
        }

        void header_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.Key("type");
            writer.Uint64(type);

            writer.Key("version");
            writer.Uint64(version);
        }

        JSON_FROM_FUNC(header_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("type");

            type = get_json_uint64_t(j, "type");

            JSON_MEMBER_OR_THROW("version");

            version = get_json_uint64_t(j, "version");
        }

        uint64_t version = 0;

      protected:
        uint64_t type = 0;
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

        void prefix_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            header_toJSON(writer);

            writer.Key("unlock_block");
            writer.Uint64(unlock_block);

            writer.Key("tx_public_key");
            tx_public_key.toJSON(writer);
        }

        JSON_FROM_FUNC(prefix_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            header_fromJSON(j);

            JSON_MEMBER_OR_THROW("unlock_block");

            unlock_block = get_json_uint64_t(j, "unlock_block");

            JSON_MEMBER_OR_THROW("tx_public_key");

            tx_public_key = get_json_string(j, "tx_public_key");
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

        void output_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.StartObject();
            {
                writer.Key("staker_id");
                staker_id.toJSON(writer);

                writer.Key("amount");
                writer.Uint64(amount);
            }
            writer.EndObject();
        }

        JSON_FROM_FUNC(output_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("staker_id");

            staker_id = get_json_string(j, "staker_id");

            JSON_MEMBER_OR_THROW("amount");

            amount = get_json_uint64_t(j, "amount");
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

        void output_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.StartObject();
            {
                writer.Key("public_ephemeral");
                public_ephemeral.toJSON(writer);

                writer.Key("amount");
                writer.Uint64(amount);

                writer.Key("commitment");
                commitment.toJSON(writer);
            }
            writer.EndObject();
        }

        JSON_FROM_FUNC(output_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("public_ephemeral");

            public_ephemeral = get_json_string(j, "public_ephemeral");

            JSON_MEMBER_OR_THROW("amount");

            amount = get_json_uint64_t(j, "amount");

            JSON_MEMBER_OR_THROW("commitment");

            commitment = get_json_string(j, "commitment");
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

        void body_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.Key("nonce");
            writer.Uint64(nonce);

            writer.Key("fee");
            writer.Uint64(fee);

            writer.Key("key_images");
            writer.StartArray();
            {
                for (const auto &key_image : key_images)
                {
                    key_image.toJSON(writer);
                }
            }
            writer.EndArray();

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

            JSON_MEMBER_OR_THROW("nonce");

            nonce = get_json_uint64_t(j, "nonce");

            JSON_MEMBER_OR_THROW("fee");

            fee = get_json_uint64_t(j, "fee");

            JSON_MEMBER_OR_THROW("key_images");

            key_images.clear();

            for (const auto &elem : get_json_array(j, "key_images"))
            {
                key_images.emplace_back(elem);
            }

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

        void suffix_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
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

            writer.Key("signatures");
            writer.StartArray();
            {
                for (const auto &signature : signatures)
                {
                    signature.toJSON(writer);
                }
            }
            writer.EndArray();

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

            JSON_MEMBER_OR_THROW("signatures");

            signatures.clear();

            for (const auto &elem : get_json_array(j, "signatures"))
            {
                signatures.emplace_back(elem);
            }

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

        void suffix_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.Key("pruning_hash");
            pruning_hash.toJSON(writer);
        }

        JSON_FROM_FUNC(suffix_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("pruning_hash");

            pruning_hash = get_json_string(j, "pruning_hash");
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

        void data_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
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

        void data_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.Key("stake_amount");
            writer.Uint64(stake_amount);

            writer.Key("candidate_public_key");
            candidate_public_key.toJSON(writer);

            writer.Key("staker_public_view_key");
            staker_public_view_key.toJSON(writer);

            writer.Key("staker_public_spend_key");
            staker_public_spend_key.toJSON(writer);
        }

        JSON_FROM_FUNC(data_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("stake_amount");

            stake_amount = get_json_uint64_t(j, "stake_amount");

            JSON_MEMBER_OR_THROW("candidate_public_key");

            candidate_public_key = get_json_string(j, "candidate_public_key");

            JSON_MEMBER_OR_THROW("staker_public_view_key");

            staker_public_view_key = get_json_string(j, "staker_public_view_key");

            JSON_MEMBER_OR_THROW("staker_public_spend_key");

            staker_public_spend_key = get_json_string(j, "staker_public_spend_key");
        }

        uint64_t stake_amount = 0;
        crypto_public_key_t candidate_public_key, staker_public_view_key, staker_public_spend_key;
    };

    struct RecallStakeTransactionData
    {
        RecallStakeTransactionData() {}

        void deserialize_data(deserializer_t &reader)
        {
            staker_id = reader.key<crypto_hash_t>();

            view_signature = reader.key<crypto_signature_t>();

            spend_signature = reader.key<crypto_signature_t>();
        }

        void serialize_data(serializer_t &writer) const
        {
            staker_id.serialize(writer);

            view_signature.serialize(writer);

            spend_signature.serialize(writer);
        }

        void data_toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.Key("stake_tx");
            staker_id.toJSON(writer);

            writer.Key("view_signature");
            view_signature.toJSON(writer);

            writer.Key("spend_signature");
            spend_signature.toJSON(writer);
        }

        JSON_FROM_FUNC(data_fromJSON)
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("stake_tx");

            staker_id = get_json_string(j, "stake_tx");

            JSON_MEMBER_OR_THROW("view_signature");

            view_signature = get_json_string(j, "view_signature");

            JSON_MEMBER_OR_THROW("spend_signature");

            spend_signature = get_json_string(j, "spend_signature");
        }

        crypto_hash_t staker_id;
        crypto_signature_t view_signature, spend_signature;
    };
} // namespace TurtleCoin::BaseTypes

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

namespace TurtleCoin::Types::Blockchain
{
    typedef TurtleCoin::BaseTypes::TransactionOutput transaction_output_t;

    typedef TurtleCoin::BaseTypes::StakerOutput staker_output_t;
} // namespace TurtleCoin::Types::Blockchain

namespace std
{
    inline ostream &operator<<(ostream &os, const TurtleCoin::Types::Blockchain::transaction_output_t &value)
    {
        os << "\tTransaction Output" << std::endl
           << "\t\tPublic Ephemeral: " << value.public_ephemeral << std::endl
           << "\t\tAmount: " << value.amount << std::endl
           << "\t\tCommitment: " << value.commitment << std::endl;

        return os;
    }

    inline ostream &operator<<(ostream &os, const TurtleCoin::Types::Blockchain::staker_output_t &value)
    {
        os << "\tStaker Output" << std::endl
           << "\t\tStaker ID: " << value.staker_id << std::endl
           << "\t\tAmount: " << value.amount << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_BLOCKCHAIN_BASE_TYPES_H
