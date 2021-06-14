// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_publisher.h"

namespace Networking
{
    ZMQPublisher::ZMQPublisher(const uint16_t &bind_port): m_bind_port(bind_port), m_running(false)
    {
        m_socket = zmq::socket_t(m_context, zmq::socket_type::pub);

        m_socket.set(zmq::sockopt::ipv6, false);

        m_socket.set(zmq::sockopt::linger, 0);

        start();
    }

    ZMQPublisher::~ZMQPublisher()
    {
        if (m_running)
        {
            stop();
        }

        close();
    }

    Error ZMQPublisher::bind()
    {
        try
        {
            m_upnp_helper =
                std::make_unique<UPNP>(m_bind_port, Configuration::Version::PROJECT_NAME + ": 0MQ Publisher");

            m_socket.bind("tcp://*:" + std::to_string(m_bind_port));

            return MAKE_ERROR(SUCCESS);
        }
        catch (const zmq::error_t &e)
        {
            return MAKE_ERROR_MSG(ZMQ_SERVER_BIND_FAILURE, e.what());
        }
    }

    void ZMQPublisher::close()
    {
        m_upnp_helper.reset();

        return m_socket.close();
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
                    m_socket.send(message.subject_msg(), zmq::send_flags::sndmore);

                    m_socket.send(message.payload_msg(), zmq::send_flags::dontwait);
                }
                catch (const zmq::error_t &e)
                {
                    // TODO: do something?
                }
            }

            THREAD_SLEEP(50);
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

    void ZMQPublisher::start()
    {
        if (m_running)
        {
            return;
        }

        m_running = true;

        m_thread_outgoing = std::thread(&ZMQPublisher::outgoing_thread, this);
    }

    void ZMQPublisher::stop()
    {
        if (m_running)
        {
            m_running = false;

            m_thread_outgoing.join();
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
