// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_BASE_TYPES_H
#define TURTLECOIN_NETWORK_BASE_TYPES_H

#include "ip_address.h"

#include <crypto.h>
#include <serializable.h>
#include <time.h>

namespace BaseTypes
{
    enum NetworkPacketTypes
    {
        NETWORK_HANDSHAKE = 1000,
        NETWORK_KEEPALIVE = 1100,
        NETWORK_PEER_EXCHANGE = 1200,
        NETWORK_DATA = 3000
    };

    struct NetworkPacketBase
    {
      public:
        uint16_t version = 1;

      protected:
        uint16_t l_type = 0;
    };
} // namespace BaseTypes

namespace Types::Network
{
    struct network_peer_t : virtual BaseTypes::IStorable
    {
        network_peer_t() {}

        network_peer_t(const ip_address_t &peer_address, const crypto_hash_t &peer_id, uint16_t peer_port):
            address(peer_address), peer_id(peer_id), port(peer_port)
        {
        }

        network_peer_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        network_peer_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(network_peer_t, fromJSON);

        void deserialize(deserializer_t &reader) override
        {
            address.deserialize(reader);

            port = reader.varint<uint16_t>();

            peer_id = reader.key<crypto_hash_t>();

            last_seen = reader.varint<uint64_t>();
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("address");

            address = ip_address_t(get_json_string(j, "address"));

            LOAD_U32_FROM_JSON(port);

            LOAD_KEY_FROM_JSON(peer_id);

            LOAD_U64_FROM_JSON(last_seen);
        }

        /**
         * Calculates the hash of the structure
         * @return
         */
        [[nodiscard]] crypto_hash_t hash() const override
        {
            const auto data = serialize();

            return Crypto::Hashing::sha3(data.data(), data.size());
        }

        void serialize(serializer_t &writer) const override
        {
            address.serialize(writer);

            writer.varint(port);

            writer.key(peer_id);

            writer.varint(last_seen);
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

        JSON_TO_FUNC(toJSON) override
        {
            writer.StartObject();
            {
                writer.Key("address");
                writer.String(address.to_string());

                U32_TO_JSON(port);

                KEY_TO_JSON(peer_id);

                U64_TO_JSON(last_seen);
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
            return 0;
        }

        ip_address_t address;
        uint16_t port = 0;
        crypto_hash_t peer_id;
        uint64_t last_seen = time(nullptr);
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::network_peer_t &value)
    {
        os << "\tPeer Entry: [" << value.size() << " bytes]" << std::endl
           << "\t\tIP Address: " << value.address.to_string() << std::endl
           << "\t\tPort: " << std::to_string(value.port) << std::endl
           << "\t\tPeer ID: " << value.peer_id << std::endl
           << "\t\tLast Seen: " << value.last_seen << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_BASE_TYPES_H
