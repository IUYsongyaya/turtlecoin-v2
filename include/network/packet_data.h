// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_PACKET_DATA_H
#define TURTLECOIN_NETWORK_PACKET_DATA_H

#include "base_types.h"

namespace TurtleCoin::Types::Network
{
    struct packet_data_t : TurtleCoin::BaseTypes::NetworkPacket
    {
        packet_data_t()
        {
            type = 3000;
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

        void serialize(serializer_t &writer) const
        {
            writer.varint(type);

            writer.varint(version);

            writer.varint(network_id.size());

            writer.bytes(network_id);

            writer.varint(payload.size());

            writer.bytes(payload);
        }

        [[nodiscard]] std::vector<uint8_t> serialize() const
        {
            auto writer = serializer_t();

            serialize(writer);

            return writer.vector();
        }

        [[nodiscard]] size_t size() const
        {
            return serialize().size();
        }

        [[nodiscard]] std::string to_string() const
        {
            const auto bytes = serialize();

            return Crypto::StringTools::to_hex(bytes.data(), bytes.size());
        }

        std::vector<uint8_t> network_id;
        std::vector<uint8_t> payload;

      private:
        void deserialize(deserializer_t &reader)
        {
            type = reader.varint<uint16_t>();

            version = reader.varint<uint16_t>();

            const auto network_bytes = reader.varint<uint64_t>();

            network_id = reader.bytes(network_bytes);

            const auto payload_bytes = reader.varint<uint64_t>();

            payload = reader.bytes(payload_bytes);
        }
    };
} // namespace TurtleCoin::Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const TurtleCoin::Types::Network::packet_data_t &value)
    {
        os << "Handshake Packet [" << value.size() << " bytes]" << std::endl
           << "\tType: " << std::to_string(value.type) << std::endl
           << "\tVersion: " << std::to_string(value.version) << std::endl
           << "\tNetwork ID: " << Crypto::StringTools::to_hex(value.network_id.data(), value.network_id.size())
           << std::endl
           << "\tPayload: " << Crypto::StringTools::to_hex(value.payload.data(), value.payload.size()) << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_PACKET_DATA_H
