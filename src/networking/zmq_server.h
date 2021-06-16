// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_ZMQ_SERVER_H
#define TURTLECOIN_NETWORKING_ZMQ_SERVER_H

#include "upnp.h"
#include "zmq_shared.h"

#include <atomic>
#include <config.h>
#include <errors.h>
#include <network/zmq_message_envelope.h>
#include <thread>
#include <tools/thread_safe_queue.h>
#include <zmq.hpp>

using namespace Types::Network;

namespace Networking
{
    /**
     * Simple ZMQ Server of the ROUTER type (for talking to multiple clients with
     * bi-directional asynchronous communication).
     *
     * Incoming messages are delivered into a queue that can be processed in the
     * order in which they were received
     *
     * Outgoing messages are queued and eventually sent unless the destination
     * client disconnected from the server
     *
     */
    class ZMQServer
    {
      public:
        /**
         * Creates a new instance and sets it up to bind to the specified port
         *
         * @param bind_port
         */
        ZMQServer(const uint16_t &bind_port = Configuration::P2P::DEFAULT_BIND_PORT);

        /**
         * Destroying the instance auto-stops the threads and closes the socket
         */
        ~ZMQServer();

        /**
         * Binds the server to the port on all available interfaces and IP
         * addresses on the machine
         *
         * @return
         */
        Error bind();

        /**
         * Returns the number of connections to the instance
         *
         * @return
         */
        size_t connections() const;

        /**
         * Returns the external IP address for the service (if detected)
         *
         * @return
         */
        std::string external_address() const;

        /**
         * Returns the identity of the server that is used in the message envelopes
         *
         * @return
         */
        crypto_hash_t identity() const;

        /**
         * Returns the current queue of incoming messages
         *
         * @return
         */
        ThreadSafeQueue<zmq_message_envelope_t> &messages();

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
         * Sends a message via the server.
         *
         * If the TO field of the message is empty, it will be broadcast to all connected clients
         * If the TO field of the message is not empty, it will be sent only to the specified client
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
         * Adds a record of the connection
         *
         * @param identity
         */
        void add_connection(const crypto_hash_t &identity);

        /**
         * Removes a record of the connection
         *
         * @param identity
         */
        void del_connection(const crypto_hash_t &identity);

        /**
         * Removes a record of the connection without aquiring lock
         *
         * @param identity
         */
        void del_connection_unsafe(const crypto_hash_t &identity);

        /**
         * The thread that reads from the ZMQ socket
         */
        void incoming_thread();

        /**
         * The thread that writes to the ZMQ socket
         */
        void outgoing_thread();

        zmq::context_t m_context;

        std::atomic<bool> m_running;

        zmq::socket_t m_socket;

        uint16_t m_bind_port;

        std::thread m_thread_incoming, m_thread_outgoing;

        mutable std::mutex m_mutex;

        std::set<crypto_hash_t> m_connections;

        crypto_hash_t m_identity;

        ThreadSafeQueue<zmq_message_envelope_t> m_incoming_msgs, m_outgoing_msgs;

        std::unique_ptr<UPNP> m_upnp_helper;

        zmq_connection_monitor m_monitor;
    };
} // namespace Networking

#endif
