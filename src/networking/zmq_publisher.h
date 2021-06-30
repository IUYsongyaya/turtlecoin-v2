// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_ZMQ_PUBLISHER_H
#define TURTLECOIN_NETWORKING_ZMQ_PUBLISHER_H

#include "upnp.h"
#include "zmq_shared.h"

#include <atomic>
#include <config.h>
#include <errors.h>
#include <network/zmq_message_envelope.h>
#include <tools/thread_safe_queue.h>
#include <zmq.hpp>

using namespace Types::Network;

namespace Networking
{
    /**
     * Simple ZMQ Server of the PUBLISHER type (for broadcasting messages to multiple
     * clients in a one-way communication architecture)
     *
     * Outgoing messages are queued and eventually sent
     *
     */
    class ZMQPublisher
    {
      public:
        /**
         * Creates a new instance and sets it up to bind to the specified port
         *
         * @param logger the shared logger
         * @param bind_port
         */
        ZMQPublisher(logger &logger, const uint16_t &bind_port = Configuration::Notifier::DEFAULT_BIND_PORT);

        /**
         * Destroying the instance auto-stops the threads and closes the socket
         */
        ~ZMQPublisher();

        /**
         * Binds the server to the port on all available interfaces and IP
         * addresses on the machine
         *
         * @return
         */
        Error bind();

        /**
         * Returns the external IP address for the service (if detected)
         *
         * @return
         */
        std::string external_address() const;

        /**
         * Returns the port of this server
         *
         * @return
         */
        uint16_t port() const;

        /**
         * Returns whether the server is running or not
         *
         * @return
         */
        bool running() const;

        /**
         * Broadcast a message to all connected subscribers
         *
         * @param message
         */
        void send(const zmq_message_envelope_t &message);

        /**
         * Returns whether UPnP is active for this instance
         *
         * @return
         */
        bool upnp_active() const;

      private:
        /**
         * The thread that writes to the ZMQ socket
         */
        void outgoing_thread();

        mutable std::mutex m_socket_mutex;

        zmq::context_t m_context;

        uint16_t m_bind_port;

        std::atomic<bool> m_running;

        zmq::socket_t m_socket;

        std::thread m_thread_outgoing;

        ThreadSafeQueue<zmq_message_envelope_t> m_outgoing_msgs;

        std::unique_ptr<UPNP> m_upnp_helper;

        zmq_connection_monitor m_monitor;

        logger m_logger;

        std::condition_variable m_stopping;
    };
} // namespace Networking

#endif
