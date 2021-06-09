// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_BASE_TYPES_H
#define TURTLECOIN_NETWORK_BASE_TYPES_H

#include <crypto.h>

namespace BaseTypes
{
    struct NetworkPacket
    {
        uint16_t type = 0;
        uint16_t version = 0;
    };
} // namespace BaseTypes

namespace Types::Network
{
    struct network_peer_t
    {
        network_peer_t() {}

        network_peer_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        network_peer_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        void serialize(serializer_t &writer) const
        {
            writer.varint(ip_address);

            writer.varint(port);

            writer.varint(peer_id.size());

            writer.bytes(peer_id);
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

        uint32_t ip_address = 0;
        uint16_t port = 0;
        std::vector<uint8_t> peer_id;

      private:
        void deserialize(deserializer_t &reader)
        {
            ip_address = reader.varint<uint32_t>();

            port = reader.varint<uint16_t>();

            const auto bytes = reader.varint<uint64_t>();

            peer_id = reader.bytes(bytes);
        }
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::network_peer_t &value)
    {
        os << "\tPeer Entry: [" << value.size() << " bytes]" << std::endl
           << "\t\tIP Address: " << std::to_string(value.ip_address) << std::endl
           << "\t\tPort: " << std::to_string(value.port) << std::endl
           << "\t\tPeer ID: " << Crypto::StringTools::to_hex(value.peer_id.data(), value.peer_id.size()) << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_BASE_TYPES_H
