// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_NETWORKING_ZMQ_SUBSCRIBER_H
#define TURTLECOIN_NETWORKING_ZMQ_SUBSCRIBER_H

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
         * Creates a new instance and auto-starts the thread
         */
        ZMQSubscriber();

        /**
         * Destroying the instance auto-stops the threads and closes the socket
         */
        ~ZMQSubscriber();

        /**
         * Closes the socket
         */
        void close();

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
         * Starts the subscriber
         *
         * NOTE: This does not connect the subscriber to anything, it only starts
         * the reading thread
         *
         */
        void start();

        /**
         * Stops the subscriber
         */
        void stop();

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

        zmq::context_t m_context;

        std::atomic<bool> m_running;

        crypto_hash_t m_identity;

        zmq::socket_t m_socket;

        std::thread m_thread_incoming;

        ThreadSafeQueue<zmq_message_envelope_t> m_incoming_msgs;
    };
} // namespace Networking

#endif
