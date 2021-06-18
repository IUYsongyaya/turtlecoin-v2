// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_subscriber.h"

#include <zmq_addon.hpp>

namespace Networking
{
    ZMQSubscriber::ZMQSubscriber(int timeout): m_identity(Crypto::random_hash()), m_running(false), m_timeout(timeout)
    {
        m_socket = zmq::socket_t(m_context, zmq::socket_type::sub);

        m_monitor.start(m_socket, ZMQ_EVENT_ALL);

        m_socket.set(zmq::sockopt::curve_serverkey, Configuration::ZMQ::SERVER_PUBLIC_KEY.c_str());

        const auto [error, public_key, secret_key] = zmq_generate_keypair();

        if (error)
        {
            throw std::runtime_error(error.to_string());
        }

        m_socket.set(zmq::sockopt::curve_publickey, public_key);

        m_socket.set(zmq::sockopt::curve_secretkey, secret_key);

        m_socket.set(zmq::sockopt::connect_timeout, timeout);

        m_socket.set(zmq::sockopt::immediate, true);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);
    }

    ZMQSubscriber::~ZMQSubscriber()
    {
        m_running = false;

        if (m_thread_incoming.joinable())
        {
            m_thread_incoming.join();
        }

        std::unique_lock lock(m_socket_mutex);

        m_socket.close();
    }

    Error ZMQSubscriber::connect(const std::string &host, const uint16_t &port)
    {
        try
        {
            std::unique_lock lock(m_socket_mutex);

            m_socket.connect("tcp://" + host + ":" + std::to_string(port));

            const auto timeout = m_monitor.cv_connected.wait_for(
                lock, std::chrono::milliseconds(Configuration::DEFAULT_CONNECTION_TIMEOUT));

            if (timeout == std::cv_status::timeout)
            {
                return MAKE_ERROR_MSG(
                    ZMQ_CLIENT_CONNECT_FAILURE, "Could not connect to " + host + ":" + std::to_string(port));
            }

            if (!m_running)
            {
                m_running = true;

                m_thread_incoming = std::thread(&ZMQSubscriber::incoming_thread, this);
            }

            return MAKE_ERROR(SUCCESS);
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_CLIENT_CONNECT_FAILURE, e.what());
        }
    }

    bool ZMQSubscriber::connected() const
    {
        return !m_monitor.connected().empty();
    }

    void ZMQSubscriber::disconnect(const std::string &host, const uint16_t &port)
    {
        try
        {
            std::unique_lock lock(m_socket_mutex);

            m_socket.disconnect("tcp://" + host + ":" + std::to_string(port));
        }
        catch (...)
        {
            // we don't care if a disconnect fails
        }
    }

    crypto_hash_t ZMQSubscriber::identity() const
    {
        return m_identity;
    }

    void ZMQSubscriber::incoming_thread()
    {
        while (m_running)
        {
            try
            {
                std::unique_lock lock(m_socket_mutex);

                zmq::multipart_t messages(m_socket, ZMQ_DONTWAIT);

                // we expect exactly two message parts and the second part should not be empty
                if (messages.size() == 2 && !messages.back().empty())
                {
                    auto message = messages.pop();

                    auto data = ZMQ_MSG_TO_VECTOR(message);

                    const auto subject = crypto_hash_t(data);

                    message = messages.pop();

                    data = ZMQ_MSG_TO_VECTOR(message);

                    auto routable_msg = zmq_message_envelope_t(m_identity, data);

                    routable_msg.subject = subject;

                    routable_msg.peer_address = ZMQ_GETS(message, "Peer-Address");

                    m_incoming_msgs.push(routable_msg);
                }
            }
            catch (const zmq::error_t &e)
            {
                // TODO: we should do something
            }

            THREAD_SLEEP();
        }
    }

    ThreadSafeQueue<zmq_message_envelope_t> &ZMQSubscriber::messages()
    {
        return m_incoming_msgs;
    }

    bool ZMQSubscriber::running() const
    {
        return m_running;
    }

    void ZMQSubscriber::subscribe(const crypto_hash_t &subject)
    {
        const auto buf = zmq::buffer(subject.data(), subject.size());

        m_socket.set(zmq::sockopt::subscribe, buf);
    }

    void ZMQSubscriber::unsubscribe(const crypto_hash_t &subject)
    {
        const auto buf = zmq::buffer(subject.data(), subject.size());

        m_socket.set(zmq::sockopt::unsubscribe, buf);
    }
} // namespace Networking
