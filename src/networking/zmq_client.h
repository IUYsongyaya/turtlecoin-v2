// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_ZMQ_CLIENT_H
#define TURTLECOIN_NETWORKING_ZMQ_CLIENT_H

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
     * Simple ZMQ Client of the DEALER type that is designed to talk to a SINGLE
     * ROUTER server (ie. ZMQServer). If connections to additional servers are
     * required a separate instance should be created.
     *
     * Incoming messages are delivered into a queue that can be processed in the
     * order in which they were received
     *
     * Outgoing messages are queued and eventually sent unless the client
     * disconnected from the server
     *
     */
    class ZMQClient
    {
      public:
        /**
         * Creates a new instance
         *
         * @param logger the shared logger
         * @param timeout in milliseconds
         */
        ZMQClient(logger &logger, int timeout = Configuration::DEFAULT_CONNECTION_TIMEOUT);

        /**
         * Destroying the instance auto-stops the threads and closes the socket
         */
        ~ZMQClient();

        /**
         * Connects the client to the specified host and port
         *
         * Both IPv4 and IPv6 are supported
         *
         * @param host
         * @param port
         * @return
         */
        Error connect(const std::string &host, const uint16_t &port = Configuration::P2P::DEFAULT_BIND_PORT);

        /**
         * Returns if the client is connected
         *
         * @return
         */
        bool connected() const;

        /**
         * Returns the identity of the client that is used in the message envelopes
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
         * Returns whether the client is running or not
         *
         * @return
         */
        bool running() const;

        /**
         * Sends a message via the client to the connected server
         *
         * TO/FROM fields of the message have no bearing on the communication
         *
         * @param message
         */
        void send(const zmq_message_envelope_t &message);

      private:
        /**
         * The thread that reads from the ZMQ socket
         */
        void incoming_thread();

        /**
         * The thread that writes to the ZMQ socket
         */
        void outgoing_thread();

        int m_timeout;

        mutable std::mutex m_connecting, m_socket_mutex;

        zmq::context_t m_context;

        std::atomic<bool> m_running;

        zmq::socket_t m_socket;

        std::thread m_thread_incoming, m_thread_outgoing;

        crypto_hash_t m_identity;

        ThreadSafeQueue<zmq_message_envelope_t> m_incoming_msgs, m_outgoing_msgs;

        zmq_connection_monitor m_monitor;

        logger m_logger;
    };
} // namespace Networking

#endif
