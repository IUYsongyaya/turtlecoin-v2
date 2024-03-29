// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_server.h"

#include "zmq_addon.hpp"

#include <tools/thread_helper.h>

namespace Networking
{
    ZMQServer::ZMQServer(logger &logger, const uint16_t &bind_port):
        m_bind_port(bind_port), m_identity(Crypto::random_hash()), m_running(false), m_logger(logger)
    {
        const auto identity = zmq::buffer(m_identity.data(), m_identity.size());

        m_socket = zmq::socket_t(m_context, zmq::socket_type::router);

        m_monitor.start(m_socket, ZMQ_EVENT_ALL);

        m_socket.set(zmq::sockopt::curve_secretkey, Configuration::ZMQ::SERVER_SECRET_KEY.c_str());

        m_socket.set(zmq::sockopt::curve_server, true);

        m_socket.set(zmq::sockopt::immediate, true);

        m_socket.set(zmq::sockopt::routing_id, identity);

        m_socket.set(zmq::sockopt::router_mandatory, true);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);
    }

    ZMQServer::~ZMQServer()
    {
        m_logger->debug("Shutting down ZMQ Server on port {0}...", m_bind_port);

        m_running = false;

        m_stopping.notify_all();

        if (m_thread_outgoing.joinable())
        {
            m_thread_outgoing.join();
        }

        m_logger->trace("ZMQ Server outgoing thread shut down successfully");

        if (m_thread_incoming.joinable())
        {
            m_thread_incoming.join();
        }

        m_logger->trace("ZMQ Server incoming thread shut down successfully");

        m_upnp_helper.reset();

        std::scoped_lock lock(m_socket_mutex);

        m_socket.close();

        m_logger->debug("ZMQ Server shutdown complete on port {0}", m_bind_port);
    }

    void ZMQServer::add_connection(const crypto_hash_t &identity)
    {
        if (!m_connections.contains(identity))
        {
            m_connections.insert(identity);

            m_logger->trace("Adding registered connection for: {0}", identity.to_string());
        }
    }

    Error ZMQServer::bind()
    {
        try
        {
            m_logger->debug("Attempting to bind ZMQ Server on *:{0}", m_bind_port);

            std::scoped_lock lock(m_socket_mutex);

            m_socket.bind("tcp://*:" + std::to_string(m_bind_port));

            if (!m_running)
            {
                m_upnp_helper = std::make_unique<UPNP>(
                    m_logger, m_bind_port, Configuration::Version::PROJECT_NAME + ": 0MQ Server");

                m_running = true;

                m_thread_incoming = std::thread(&ZMQServer::incoming_thread, this);

                m_thread_outgoing = std::thread(&ZMQServer::outgoing_thread, this);
            }

            m_logger->debug("ZMQ Server bound on *:{0}", m_bind_port);

            return MAKE_ERROR(SUCCESS);
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_BIND_ERROR, e.what());
        }
    }

    size_t ZMQServer::connections() const
    {
        return m_connections.size();
    }

    void ZMQServer::del_connection(const crypto_hash_t &identity)
    {
        if (m_connections.contains(identity))
        {
            m_connections.erase(identity);
        }

        m_logger->trace("Deleting registered connection for: {0}", identity.to_string());
    }

    std::string ZMQServer::external_address() const
    {
        if (!m_upnp_helper)
        {
            return std::string();
        }

        return m_upnp_helper->external_address();
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
                std::scoped_lock lock(m_socket_mutex);

                zmq::multipart_t messages(m_socket, ZMQ_DONTWAIT);

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

                    m_logger->trace(
                        "Message received from {0}: {1}",
                        routable_msg.peer_address,
                        Crypto::StringTools::to_hex(routable_msg.payload.data(), routable_msg.payload.size()));
                }
            }
            catch (const zmq::error_t &e)
            {
                m_logger->trace("Could not read incoming ZMQ message: {0}", e.what());
            }

            if (thread_sleep(m_stopping))
            {
                break;
            }
        }
    }

    ThreadSafeQueue<zmq_message_envelope_t> &ZMQServer::messages()
    {
        return m_incoming_msgs;
    }

    uint16_t ZMQServer::port() const
    {
        return m_bind_port;
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
                    for (const auto &to : m_connections)
                    {
                        try
                        {
                            message.to = to;

                            std::scoped_lock socket_lock(m_socket_mutex);

                            m_socket.send(message.to_msg(), zmq::send_flags::sndmore);

                            m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);

                            m_logger->trace(
                                "Message sent to {0}: {1}",
                                message.to.to_string(),
                                Crypto::StringTools::to_hex(message.payload.data(), message.payload.size()));
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
                        std::scoped_lock socket_lock(m_socket_mutex);

                        m_socket.send(message.to_msg(), zmq::send_flags::sndmore);

                        m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);

                        m_logger->trace(
                            "Message sent to {0}: {1}",
                            message.to.to_string(),
                            Crypto::StringTools::to_hex(message.payload.data(), message.payload.size()));
                    }
                    catch (const zmq::error_t &e)
                    {
                        del_connection(message.to);
                    }
                }
            }

            if (thread_sleep(m_stopping))
            {
                break;
            }
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

    bool ZMQServer::upnp_active() const
    {
        if (!m_upnp_helper)
        {
            return false;
        }

        return m_upnp_helper->active();
    }
} // namespace Networking
