// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_ZMQ_ROUTABLE_MSG_H
#define TURTLECOIN_ZMQ_ROUTABLE_MSG_H

#include <crypto.h>
#include <zmq.hpp>

#ifndef THREAD_SLEEP
#define THREAD_SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))
#endif

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

        zmq_message_envelope_t(const std::vector<uint8_t> &payload): payload(payload) {}

        template<typename T> zmq_message_envelope_t(const T &payload): payload(payload.serialize()) {}

        zmq_message_envelope_t(const crypto_hash_t &to, const crypto_hash_t &from): to(to), from(from) {}

        zmq_message_envelope_t(const crypto_hash_t &to, const std::vector<uint8_t> &payload): to(to), payload(payload)
        {
        }

        template<typename T>
        zmq_message_envelope_t(const crypto_hash_t &to, const T &payload): to(to), payload(payload.serialize())
        {
        }

        zmq_message_envelope_t(const crypto_hash_t &to, const crypto_hash_t &from, const std::vector<uint8_t> &payload):
            to(to), from(from), payload(payload)
        {
        }

        zmq::message_t from_msg() const
        {
            return zmq::message_t(from.data(), from.size());
        }

        zmq::message_t payload_msg() const
        {
            return zmq::message_t(payload.data(), payload.size());
        }

        size_t size() const
        {
            return to.size() + from.size() + peer_address.size() + payload.size();
        }

        zmq::message_t subject_msg() const
        {
            return zmq::message_t(subject.data(), subject.size());
        }

        zmq::message_t to_msg() const
        {
            return zmq::message_t(to.data(), to.size());
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
        os << "ZMQ Message Envelope [" << value.size() << " bytes]" << std::endl
           << "To: " << value.to << std::endl
           << "From: " << value.from << std::endl
           << "Subject: " << value.subject << std::endl
           << "Peer Address: " << value.peer_address << std::endl
           << "Payload [" << value.payload.size()
           << " bytes]: " << Crypto::StringTools::to_hex(value.payload.data(), value.payload.size()) << std::endl;

        return os;
    }
} // namespace std

#endif
