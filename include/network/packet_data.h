// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_PACKET_DATA_H
#define TURTLECOIN_NETWORK_PACKET_DATA_H

#include "base_types.h"

namespace Types::Network
{
    struct packet_data_t : BaseTypes::NetworkPacketBase, virtual BaseTypes::IStorable
    {
        packet_data_t()
        {
            l_type = BaseTypes::NETWORK_DATA;
        }

        packet_data_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        packet_data_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        packet_data_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(packet_data_t, fromJSON);

        void deserialize(deserializer_t &reader) override
        {
            l_type = reader.varint<uint16_t>();

            version = reader.varint<uint16_t>();

            const auto network_bytes = reader.varint<uint64_t>();

            network_id = reader.bytes(network_bytes);

            const auto payload_bytes = reader.varint<uint64_t>();

            payload = reader.bytes(payload_bytes);
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("type");

            l_type = get_json_uint32_t(j, "type");

            LOAD_U32_FROM_JSON(version);

            JSON_MEMBER_OR_THROW("network_id");

            network_id = Crypto::StringTools::from_hex(get_json_string(j, "network_id"));

            JSON_MEMBER_OR_THROW("payload");

            payload = Crypto::StringTools::from_hex(get_json_string(j, "payload"));
        }

        /**
         * Calculates the hash of the structure
         * @return
         */
        [[nodiscard]] crypto_hash_t hash() const override
        {
            return serialize();
        }

        void serialize(serializer_t &writer) const override
        {
            writer.varint(l_type);

            writer.varint(version);

            writer.varint(network_id.size());

            writer.bytes(network_id);

            writer.varint(payload.size());

            writer.bytes(payload);
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const override
        {
            auto writer = serializer_t();

            serialize(writer);

            return writer.vector();
        }

        [[nodiscard]] size_t size() const override
        {
            return serialize().size();
        }

        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const override
        {
            writer.StartObject();
            {
                writer.Key("type");
                writer.Uint(l_type);

                U32_TO_JSON(version);

                writer.Key("network_id");
                writer.String(Crypto::StringTools::to_hex(network_id.data(), network_id.size()));

                writer.Key("payload");
                writer.String(Crypto::StringTools::to_hex(payload.data(), payload.size()));
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
            return l_type;
        }

        std::vector<uint8_t> network_id;
        std::vector<uint8_t> payload;
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::packet_data_t &value)
    {
        os << "Handshake Packet [" << value.size() << " bytes]" << std::endl
           << "\tType: " << std::to_string(value.type()) << std::endl
           << "\tVersion: " << std::to_string(value.version) << std::endl
           << "\tNetwork ID: " << Crypto::StringTools::to_hex(value.network_id.data(), value.network_id.size())
           << std::endl
           << "\tPayload: " << Crypto::StringTools::to_hex(value.payload.data(), value.payload.size()) << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_PACKET_DATA_H
