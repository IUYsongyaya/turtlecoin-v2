// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_BASE_TYPES_H
#define TURTLECOIN_NETWORK_BASE_TYPES_H

#include <crypto.h>
#include <serializable.h>

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
            ip_address = reader.varint<uint32_t>();

            port = reader.varint<uint16_t>();

            peer_id = reader.key<crypto_hash_t>();
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            LOAD_U32_FROM_JSON(ip_address);

            LOAD_U32_FROM_JSON(port);

            LOAD_KEY_FROM_JSON(peer_id);
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
            writer.varint(ip_address);

            writer.varint(port);

            writer.key(peer_id);
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
                U32_TO_JSON(ip_address);

                U32_TO_JSON(port);

                KEY_TO_JSON(peer_id);
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

        uint32_t ip_address = 0;
        uint16_t port = 0;
        crypto_hash_t peer_id;
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::network_peer_t &value)
    {
        os << "\tPeer Entry: [" << value.size() << " bytes]" << std::endl
           << "\t\tIP Address: " << std::to_string(value.ip_address) << std::endl
           << "\t\tPort: " << std::to_string(value.port) << std::endl
           << "\t\tPeer ID: " << value.peer_id << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_BASE_TYPES_H
