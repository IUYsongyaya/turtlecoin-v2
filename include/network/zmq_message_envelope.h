// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_ZMQ_ROUTABLE_MSG_H
#define TURTLECOIN_ZMQ_ROUTABLE_MSG_H

#include <crypto.h>
#include <zmq.hpp>

#define ZMQ_GETS(payload, property) std::string((payload).gets(property))
#define ZMQ_MSG_TO_VECTOR(message) \
    std::vector<uint8_t>((message).data<uint8_t>(), (message).data<uint8_t>() + (message).size())
#define ZMQ_IDENT_TO_HASH(ident) crypto_hash_t(std::vector<uint8_t>((ident).data(), (ident).data() + (ident).size()))

namespace Types::Network
{
    struct zmq_message_envelope_t
    {
        zmq_message_envelope_t() {}

        zmq_message_envelope_t(const crypto_hash_t &to): to(to) {}

        zmq_message_envelope_t(std::vector<uint8_t> payload): payload(std::move(payload)) {}

        template<typename T> zmq_message_envelope_t(const T &payload): payload(payload.serialize()) {}

        zmq_message_envelope_t(const crypto_hash_t &to, const crypto_hash_t &from): to(to), from(from) {}

        zmq_message_envelope_t(const crypto_hash_t &to, std::vector<uint8_t> payload):
            to(to), payload(std::move(payload))
        {
        }

        template<typename T>
        zmq_message_envelope_t(const crypto_hash_t &to, const T &payload): to(to), payload(payload.serialize())
        {
        }

        zmq_message_envelope_t(const crypto_hash_t &to, const crypto_hash_t &from, std::vector<uint8_t> payload):
            to(to), from(from), payload(std::move(payload))
        {
        }

        [[nodiscard]] zmq::message_t from_msg() const
        {
            return zmq::message_t(from.data(), from.size());
        }

        [[nodiscard]] zmq::message_t payload_msg() const
        {
            return zmq::message_t(payload.data(), payload.size());
        }

        [[nodiscard]] size_t size() const
        {
            return to.size() + from.size() + peer_address.size() + payload.size();
        }

        [[nodiscard]] zmq::message_t subject_msg() const
        {
            return zmq::message_t(subject.data(), subject.size());
        }

        [[nodiscard]] zmq::message_t to_msg() const
        {
            return zmq::message_t(to.data(), to.size());
        }

        [[nodiscard]] std::string to_string() const
        {
            std::stringstream ss;

            ss << "ZMQ Message Envelope [" << size() << " bytes]" << std::endl
               << "To: " << to << std::endl
               << "From: " << from << std::endl
               << "Subject: " << subject << std::endl
               << "Peer Address: " << peer_address << std::endl
               << "Payload [" << payload.size()
               << " bytes]: " << Crypto::StringTools::to_hex(payload.data(), payload.size()) << std::endl;

            return ss.str();
        }

        crypto_hash_t to, from, subject;

        std::string peer_address;

        std::vector<uint8_t> payload;
    };
} // namespace Types::Network

namespace std
{
    inline ostream &operator<<(ostream &os, const Types::Network::zmq_message_envelope_t &value)
    {
        os << value.to_string();

        return os;
    }
} // namespace std

#endif
