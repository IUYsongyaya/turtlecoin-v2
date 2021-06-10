// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORK_PACKET_KEEPALIVE_H
#define TURTLECOIN_NETWORK_PACKET_KEEPALIVE_H

#include "base_types.h"

namespace Types::Network
{
    struct packet_keepalive_t : BaseTypes::NetworkPacketBase, virtual BaseTypes::IStorable
    {
        packet_keepalive_t()
        {
            l_type = BaseTypes::NETWORK_KEEPALIVE;
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

        JSON_OBJECT_CONSTRUCTORS(packet_keepalive_t, fromJSON);

        void deserialize(deserializer_t &reader) override
        {
            l_type = reader.varint<uint16_t>();

            version = reader.varint<uint16_t>();
        }

        JSON_FROM_FUNC(fromJSON) override
        {
            JSON_OBJECT_OR_THROW();

            JSON_MEMBER_OR_THROW("type");

            l_type = get_json_uint32_t(j, "type");

            LOAD_U32_FROM_JSON(version);
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
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::packet_keepalive_t &value)
    {
        os << "Keepalive Packet [" << value.size() << " bytes]" << std::endl
           << "\tType: " << std::to_string(value.type()) << std::endl
           << "\tVersion: " << std::to_string(value.version) << std::endl;

        return os;
    }
} // namespace std

#endif // TURTLECOIN_NETWORK_PACKET_KEEPALIVE_H
