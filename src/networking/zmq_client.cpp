// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_client.h"

#include <zmq_addon.hpp>

namespace Networking
{
    ZMQClient::ZMQClient(int timeout): m_identity(Crypto::random_hash()), m_running(false), m_timeout(timeout)
    {
        const auto identity = zmq::buffer(m_identity.data(), m_identity.size());

        m_socket = zmq::socket_t(m_context, zmq::socket_type::dealer);

        m_monitor.start(m_socket, ZMQ_EVENT_ALL);

        m_socket.set(zmq::sockopt::connect_timeout, timeout);

        m_socket.set(zmq::sockopt::routing_id, identity);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);

        m_socket.set(zmq::sockopt::probe_router, true);
    }

    ZMQClient::~ZMQClient()
    {
        m_running = false;

        if (m_thread_outgoing.joinable())
        {
            m_thread_outgoing.join();
        }

        if (m_thread_incoming.joinable())
        {
            m_thread_incoming.join();
        }

        std::scoped_lock lock(m_socket_mutex);

        m_socket.close();
    }

    Error ZMQClient::connect(const std::string &host, const uint16_t &port)
    {
        try
        {
            std::unique_lock lock(m_connecting);

            std::scoped_lock socket_lock(m_socket_mutex);

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

                m_thread_incoming = std::thread(&ZMQClient::incoming_thread, this);

                m_thread_outgoing = std::thread(&ZMQClient::outgoing_thread, this);
            }

            return MAKE_ERROR(SUCCESS);
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_CLIENT_CONNECT_FAILURE, e.what());
        }
    }

    bool ZMQClient::connected() const
    {
        return !m_monitor.connected().empty();
    }

    crypto_hash_t ZMQClient::identity() const
    {
        return m_identity;
    }

    void ZMQClient::incoming_thread()
    {
        while (m_running)
        {
            try
            {
                std::scoped_lock lock(m_socket_mutex);

                zmq::multipart_t messages(m_socket, ZMQ_DONTWAIT);

                // we expect exactly one message part and it should not be empty
                if (messages.size() == 1 && !messages.front().empty())
                {
                    auto message = messages.pop();

                    const auto ident_data = ZMQ_GETS(message, "Identity");

                    const auto from = ZMQ_IDENT_TO_HASH(ident_data);

                    const auto data = ZMQ_MSG_TO_VECTOR(message);

                    auto routable_msg = zmq_message_envelope_t(m_identity, from, data);

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

    ThreadSafeQueue<zmq_message_envelope_t> &ZMQClient::messages()
    {
        return m_incoming_msgs;
    }

    void ZMQClient::outgoing_thread()
    {
        while (m_running)
        {
            while (!m_outgoing_msgs.empty())
            {
                // allow for early breakout if stopping
                if (!m_running)
                {
                    break;
                }

                auto message = m_outgoing_msgs.pop();

                // skip empty messages
                if (message.payload.empty())
                {
                    continue;
                }

                try
                {
                    std::scoped_lock lock(m_socket_mutex);

                    m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);
                }
                catch (const zmq::error_t &e)
                {
                    std::cout << e.what() << std::endl;
                    // TODO: Do something
                }
            }

            THREAD_SLEEP();
        }
    }

    bool ZMQClient::running() const
    {
        return m_running;
    }

    void ZMQClient::send(const zmq_message_envelope_t &message)
    {
        if (!message.payload.empty() && m_running)
        {
            m_outgoing_msgs.push(message);
        }
    }
} // namespace Networking
