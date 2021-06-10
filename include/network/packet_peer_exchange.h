// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_PACKET_PEER_EXCHANGE_H
#define TURTLECOIN_NETWORK_PACKET_PEER_EXCHANGE_H

#include "base_types.h"

namespace Types::Network
{
    struct packet_peer_exchange_t : BaseTypes::NetworkPacketBase, virtual BaseTypes::IStorable
    {
        packet_peer_exchange_t()
        {
            l_type = BaseTypes::NETWORK_PEER_EXCHANGE;
        }

        packet_peer_exchange_t(deserializer_t &reader)
        {
            deserialize(reader);
        }

        packet_peer_exchange_t(std::initializer_list<uint8_t> input)
        {
            std::vector<uint8_t> data(input);

            auto reader = deserializer_t(data);

            deserialize(reader);
        }

        packet_peer_exchange_t(const std::vector<uint8_t> &data)
        {
            deserializer_t reader(data);

            deserialize(reader);
        }

        JSON_OBJECT_CONSTRUCTORS(packet_peer_exchange_t, fromJSON);

        void deserialize(deserializer_t &reader) override
        {
            l_type = reader.varint<uint16_t>();

            version = reader.varint<uint16_t>();

            const auto peer_count = reader.varint<uint64_t>();

            peers.clear();

            for (size_t i = 0; i < peer_count; ++i)
            {
                const auto peer = network_peer_t(reader);

                peers.push_back(peer);
            }
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("type");

            l_type = get_json_uint32_t(j, "type");

            LOAD_U32_FROM_JSON(version);

            JSON_MEMBER_OR_THROW("peers");

            peers.clear();

            for (const auto &elem : get_json_array(j, "peers"))
            {
                peers.emplace_back(elem);
            }
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

            writer.varint(peers.size());

            for (const auto &peer : peers)
            {
                peer.serialize(writer);
            }
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
                writer.Key("type");
                writer.Uint(l_type);

                U32_TO_JSON(version);

                writer.Key("peers");
                writer.StartArray();
                {
                    for (const auto &peer : peers)
                    {
                        peer.toJSON(writer);
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

        [[nodiscard]] uint64_t type() const override
        {
            return l_type;
        }

        std::vector<network_peer_t> peers;
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::packet_peer_exchange_t &value)
    {
        os << "Handshake Packet [" << value.size() << " bytes]" << std::endl
           << "\tType: " << std::to_string(value.type()) << std::endl
           << "\tVersion: " << std::to_string(value.version) << std::endl
           << "\tPeers: " << std::endl;

        for (const auto &peer : value.peers)
        {
            os << peer << std::endl;
        }

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_PACKET_PEER_EXCHANGE_H
