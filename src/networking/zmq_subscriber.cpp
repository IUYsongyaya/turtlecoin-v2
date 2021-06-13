// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "zmq_subscriber.h"

#include <zmq_addon.hpp>

namespace Networking
{
    ZMQSubscriber::ZMQSubscriber(): m_identity(Crypto::random_hash()), m_running(false)
    {
        m_socket = zmq::socket_t(m_context, zmq::socket_type::sub);

        m_socket.set(zmq::sockopt::ipv6, true);

        m_socket.set(zmq::sockopt::linger, 0);

        start();
    }

    ZMQSubscriber::~ZMQSubscriber()
    {
        if (m_running)
        {
            stop();
        }

        m_socket.close();
    }

    void ZMQSubscriber::close()
    {
        return m_socket.close();
    }

    Error ZMQSubscriber::connect(const std::string &host, const uint16_t &port)
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

    void ZMQSubscriber::disconnect(const std::string &host, const uint16_t &port)
    {
        try
        {
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
                zmq::multipart_t messages(m_socket);

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

            THREAD_SLEEP(50);
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

    void ZMQSubscriber::start()
    {
        if (m_running)
        {
            return;
        }

        m_running = true;

        m_thread_incoming = std::thread(&ZMQSubscriber::incoming_thread, this);
    }

    void ZMQSubscriber::stop()
    {
        if (m_running)
        {
            m_running = false;

            m_thread_incoming.join();
        }
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
