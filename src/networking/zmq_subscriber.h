// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_ZMQ_SUBSCRIBER_H
#define TURTLECOIN_NETWORKING_ZMQ_SUBSCRIBER_H

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
     * Simple ZMQ Client of the SUBSCRIBER type that is designed to listen to MULTIPLE
     * PUBLISHERs (ie. ZMQPublisher). If connections to additional servers are required
     * multiple calls to connect() may be made. Incoming messages are fair-weight
     * queued by the underlying ZMQ library.
     *
     * Incoming messages are delivered into a queue that can be processed in the
     * order in which they were received.
     *
     */
    class ZMQSubscriber
    {
      public:
        /**
         * Creates a new instance
         *
         * @param logger the shared logger
         * @param timeout in milliseconds
         */
        ZMQSubscriber(logger &logger, int timeout = Configuration::DEFAULT_CONNECTION_TIMEOUT);

        /**
         * Destroying the instance auto-stops the threads and closes the socket
         */
        ~ZMQSubscriber();

        /**
         * Connects the subscriber to the specified host and port
         *
         * Both IPv4 and IPv6 are supported
         *
         * @param host
         * @param port
         * @return
         */
        Error connect(const std::string &host, const uint16_t &port = Configuration::Notifier::DEFAULT_BIND_PORT);

        /**
         * Returns if the client is connected
         *
         * @return
         */
        bool connected() const;

        /**
         * Disconnects the subscriber from the specified host and port
         *
         * Both IPv4 and IPv6 are supported
         *
         * @param host
         * @param port
         */
        void disconnect(const std::string &host, const uint16_t &port = Configuration::Notifier::DEFAULT_BIND_PORT);

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
         * Returns whether the subscriber is running or not
         *
         * @return
         */
        bool running() const;

        /**
         * Subscribe to messages of the specified subject
         *
         * @param subject
         */
        void subscribe(const crypto_hash_t &subject = {});

        /**
         * Unsubscribe from messages of the specified subject
         *
         * @param subject
         */
        void unsubscribe(const crypto_hash_t &subject = {});

      private:
        /**
         * The thread that reads from the ZMQ socket
         */
        void incoming_thread();

        int m_timeout;

        mutable std::mutex m_socket_mutex;

        zmq::context_t m_context;

        std::atomic<bool> m_running;

        crypto_hash_t m_identity;

        zmq::socket_t m_socket;

        std::thread m_thread_incoming;

        ThreadSafeQueue<zmq_message_envelope_t> m_incoming_msgs;

        zmq_connection_monitor m_monitor;

        logger m_logger;
    };
} // namespace Networking

#endif
