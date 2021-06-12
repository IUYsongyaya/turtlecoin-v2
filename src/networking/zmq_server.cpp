// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_server.h"

namespace Networking
{
    ZMQServer::ZMQServer(uint16_t bind_port): m_bind_port(bind_port), m_identity(Crypto::random_hash())
    {
        const auto identity = zmq::buffer(m_identity.data(), m_identity.size());

        m_socket = zmq::socket_t(m_context, zmq::socket_type::router);

        m_socket.set(zmq::sockopt::routing_id, identity);

        m_socket.set(zmq::sockopt::router_mandatory, true);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);

        start();
    }

    ZMQServer::~ZMQServer()
    {
        if (m_running)
        {
            stop();
        }

        m_socket.close();
    }

    void ZMQServer::add_connection(const crypto_hash_t &identity)
    {
        std::scoped_lock lock(m_mutex);

        if (m_connections.find(identity) == m_connections.end())
        {
            m_connections.insert(identity);
        }
    }

    Error ZMQServer::bind()
    {
        try
        {
            m_socket.bind("tcp://*:" + std::to_string(m_bind_port));

            return SUCCESS;
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_SERVER_BIND_FAILURE, e.what());
        }
    }

    void ZMQServer::close()
    {
        return m_socket.close();
    }

    size_t ZMQServer::connections() const
    {
        std::scoped_lock lock(m_mutex);

        return m_connections.size();
    }

    void ZMQServer::del_connection(const crypto_hash_t &identity)
    {
        std::scoped_lock lock(m_mutex);

        del_connection_unsafe(identity);
    }

    void ZMQServer::del_connection_unsafe(const crypto_hash_t &identity)
    {
        if (m_connections.find(identity) != m_connections.end())
        {
            m_connections.erase(identity);
        }
    }

    crypto_hash_t ZMQServer::identity() const
    {
        return m_identity;
    }

    void ZMQServer::incoming_thread()
    {
        while (m_running)
        {
            try
            {
                zmq::multipart_t messages(m_socket);

                // we expect exactly two message parts and the second part should not be empty
                if (messages.size() == 2 && !messages.back().empty())
                {
                    auto message = messages.pop();

                    auto data = ZMQ_MSG_TO_VECTOR(message);

                    const auto from = crypto_hash_t(data);

                    add_connection(from);

                    message = messages.pop();

                    data = ZMQ_MSG_TO_VECTOR(message);

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

    ThreadSafeQueue<zmq_message_envelope_t> &ZMQServer::messages()
    {
        return m_incoming_msgs;
    }

    void ZMQServer::outgoing_thread()
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

                // messages without a destination are BROADCAST messages
                if (message.to.empty())
                {
                    std::scoped_lock lock(m_mutex);

                    for (const auto &to : m_connections)
                    {
                        try
                        {
                            message.to = to;

                            m_socket.send(message.to_msg(), zmq::send_flags::sndmore);

                            m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);
                        }
                        catch (const zmq::error_t &e)
                        {
                            del_connection(to);
                        }
                    }
                }
                else // otherwise, we have a specific destination in mind
                {
                    try
                    {
                        m_socket.send(message.to_msg(), zmq::send_flags::sndmore);

                        m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);
                    }
                    catch (const zmq::error_t &e)
                    {
                        del_connection(message.to);
                    }
                }
            }

            THREAD_SLEEP(50);
        }
    }

    bool ZMQServer::running() const
    {
        return m_running;
    }

    void ZMQServer::send(const zmq_message_envelope_t &message)
    {
        if (!message.payload.empty() && m_running)
        {
            m_outgoing_msgs.push(message);
        }
    }

    void ZMQServer::start()
    {
        if (m_running)
        {
            return;
        }

        m_running = true;

        m_thread_incoming = std::thread(&ZMQServer::incoming_thread, this);

        m_thread_outgoing = std::thread(&ZMQServer::outgoing_thread, this);
    }

    void ZMQServer::stop()
    {
        if (m_running)
        {
            m_running = false;

            m_thread_outgoing.join();

            m_thread_incoming.join();
        }
    }
} // namespace Networking
