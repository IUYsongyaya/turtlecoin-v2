// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_PACKET_KEEPALIVE_H
#define TURTLECOIN_NETWORK_PACKET_KEEPALIVE_H

#include "base_types.h"

namespace TurtleCoin::Types::Network
{
    struct packet_keepalive_t : TurtleCoin::BaseTypes::NetworkPacket
    {
        packet_keepalive_t()
        {
            type = 1100;
        }

        packet_keepalive_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        packet_keepalive_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        packet_keepalive_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        void serialize(serializer_t &writer) const
        {
            writer.varint(type);

            writer.varint(version);
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

      private:
        void deserialize(deserializer_t &reader)
        {
            type = reader.varint<uint16_t>();

            version = reader.varint<uint16_t>();
        }
    };
} // namespace TurtleCoin::Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const TurtleCoin::Types::Network::packet_keepalive_t &value)
    {
        os << "Keepalive Packet [" << value.size() << " bytes]" << std::endl
           << "\tType: " << std::to_string(value.type) << std::endl
           << "\tVersion: " << std::to_string(value.version) << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_PACKET_KEEPALIVE_H
