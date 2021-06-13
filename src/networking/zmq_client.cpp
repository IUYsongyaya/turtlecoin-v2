// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_client.h"

#include <zmq_addon.hpp>

namespace Networking
{
    ZMQClient::ZMQClient(): m_identity(Crypto::random_hash()), m_running(false)
    {
        const auto identity = zmq::buffer(m_identity.data(), m_identity.size());

        m_socket = zmq::socket_t(m_context, zmq::socket_type::dealer);

        m_socket.set(zmq::sockopt::routing_id, identity);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);

        m_socket.set(zmq::sockopt::probe_router, true);

        start();
    }

    ZMQClient::~ZMQClient()
    {
        if (m_running)
        {
            stop();
        }

        m_socket.close();
    }

    void ZMQClient::close()
    {
        return m_socket.close();
    }

    Error ZMQClient::connect(const std::string &host, const uint16_t &port)
    {
        if (!m_running)
        {
            return MAKE_ERROR_MSG(ZMQ_CLIENT_CONNECT_FAILURE, "Client not running.");
        }

        try
        {
            m_socket.connect("tcp://" + host + ":" + std::to_string(port));

            return MAKE_ERROR(SUCCESS);
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_CLIENT_CONNECT_FAILURE, e.what());
        }
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
                zmq::multipart_t messages(m_socket);

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

            THREAD_SLEEP(50);
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
                    m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);
                }
                catch (const zmq::error_t &e)
                {
                    std::cout << e.what() << std::endl;
                    // TODO: Do something
                }
            }

            THREAD_SLEEP(50);
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

    void ZMQClient::start()
    {
        if (m_running)
        {
            return;
        }

        m_running = true;

        m_thread_incoming = std::thread(&ZMQClient::incoming_thread, this);

        m_thread_outgoing = std::thread(&ZMQClient::outgoing_thread, this);
    }

    void ZMQClient::stop()
    {
        if (m_running)
        {
            m_running = false;

            m_thread_outgoing.join();

            m_thread_incoming.join();
        }
    }
} // namespace Networking
