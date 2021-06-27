// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_publisher.h"

#include <tools/thread_helper.h>

namespace Networking
{
    ZMQPublisher::ZMQPublisher(logger &logger, const uint16_t &bind_port):
        m_bind_port(bind_port), m_running(false), m_logger(logger)
    {
        const auto identity = Crypto::random_hash();

        m_socket = zmq::socket_t(m_context, zmq::socket_type::pub);

        m_monitor.start(m_socket, ZMQ_EVENT_ALL);

        m_socket.set(zmq::sockopt::curve_secretkey, Configuration::ZMQ::SERVER_SECRET_KEY.c_str());

        m_socket.set(zmq::sockopt::curve_server, true);

        m_socket.set(zmq::sockopt::immediate, true);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);
    }

    ZMQPublisher::~ZMQPublisher()
    {
        m_logger->info("Shutting down ZMQ Publisher on port {0}...", m_bind_port);

        m_running = false;

        m_stopping.notify_all();

        if (m_thread_outgoing.joinable())
        {
            m_thread_outgoing.join();
        }

        m_logger->debug("ZMQ Publisher outgoing thread shut down successfully");

        m_upnp_helper.reset();

        std::scoped_lock lock(m_socket_mutex);

        m_socket.close();

        m_logger->info("ZMQ Publisher shutdown complete on port {0}", m_bind_port);
    }

    Error ZMQPublisher::bind()
    {
        try
        {
            m_logger->info("Attempting to bind ZMQ Publisher on *:{0}", m_bind_port);

            std::scoped_lock lock(m_socket_mutex);

            m_socket.bind("tcp://*:" + std::to_string(m_bind_port));

            if (!m_running)
            {
                m_upnp_helper = std::make_unique<UPNP>(
                    m_logger, m_bind_port, Configuration::Version::PROJECT_NAME + ": 0MQ Publisher");

                m_running = true;

                m_thread_outgoing = std::thread(&ZMQPublisher::outgoing_thread, this);
            }

            m_logger->info("ZMQ Publisher bound on *:{0}", m_bind_port);

            return MAKE_ERROR(SUCCESS);
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_SERVER_BIND_FAILURE, e.what());
        }
    }

    std::string ZMQPublisher::external_address() const
    {
        if (!m_upnp_helper)
        {
            return std::string();
        }

        return m_upnp_helper->external_address();
    }

    uint16_t ZMQPublisher::port() const
    {
        return m_bind_port;
    }

    void ZMQPublisher::outgoing_thread()
    {
        while (true)
        {
            while (!m_outgoing_msgs.empty())
            {
                auto message = m_outgoing_msgs.pop();

                // skip empty messages
                if (message.payload.empty())
                {
                    continue;
                }

                try
                {
                    std::scoped_lock lock(m_socket_mutex);

                    m_socket.send(message.subject_msg(), zmq::send_flags::sndmore);

                    m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);
                }
                catch (const zmq::error_t &e)
                {
                    m_logger->warn("Could not send ZMQ message: {0}", e.what());
                }
            }

            if (thread_sleep(m_stopping))
            {
                break;
            }
        }
    }

    bool ZMQPublisher::running() const
    {
        return m_running;
    }

    void ZMQPublisher::send(const zmq_message_envelope_t &message)
    {
        if (!message.payload.empty() && m_running)
        {
            m_outgoing_msgs.push(message);
        }
    }

    bool ZMQPublisher::upnp_active() const
    {
        if (!m_upnp_helper)
        {
            return false;
        }

        return m_upnp_helper->active();
    }
} // namespace Networking
