// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_P2P_NETWORK_NODE_H
#define TURTLECOIN_P2P_NETWORK_NODE_H

#include "peer_database.h"

#include <condition_variable>
#include <zmq_client.h>
#include <zmq_server.h>

namespace P2P
{
    class NetworkNode
    {
      public:
        /**
         * Constructs a new instance of the Network Node object
         *
         * @param logger
         * @param path
         * @param bind_port
         * @param seed_mode
         */
        NetworkNode(logger &logger, const std::string &path, const uint16_t &bind_port, bool seed_mode = false);

        ~NetworkNode();

        /**
         * Returns the number of incoming connections
         *
         * @return
         */
        size_t incoming_connections() const;

        /**
         * Returns the number of outgoing connections
         *
         * @return
         */
        size_t outgoing_connections() const;

        /**
         * Returns our peer ID
         *
         * @return
         */
        crypto_hash_t peer_id() const;

        /**
         * Returns the instance of the peer database
         *
         * @return
         */
        std::shared_ptr<PeerDB> peers() const;

        /**
         * Returns if the P2P network node is running
         *
         * @return
         */
        bool running() const;

        /**
         * Sends the message out to all of the connected network peers
         *
         * @param message
         */
        void send(const zmq_message_envelope_t &message);

        /**
         * Starts the P2P network node
         *
         * @return
         */
        Error start();

      private:
        /**
         * Builds a handshake packet
         *
         * @return
         */
        packet_handshake_t build_handshake() const;

        /**
         * Builds the network peer list for exchange/handshake
         *
         * @return
         */
        std::vector<network_peer_t> build_peer_list() const;

        /**
         * Connects a new client instance to a server
         *
         * @param host
         * @param port
         * @return
         */
        Error connect(const std::string &host, const uint16_t &port);

        /**
         * The connection manager thread
         */
        void connection_manager();

        /**
         * Handles all incoming messages from both the server and the clients
         *
         * @param message
         * @param is_server
         */
        void handle_incoming_message(const zmq_message_envelope_t &message, bool is_server = false);

        /**
         * Handles the handshake packet
         *
         * @param from
         * @param peer_address
         * @param packet
         * @param is_server
         */
        void handle_packet(
            const crypto_hash_t &from,
            const std::string &peer_address,
            const Types::Network::packet_handshake_t &packet,
            bool is_server = false);

        /**
         * Handles a peer exchange packet
         *
         * @param from
         * @param peer_address
         * @param packet
         * @param is_server
         */
        void handle_packet(
            const crypto_hash_t &from,
            const std::string &peer_address,
            const Types::Network::packet_peer_exchange_t &packet,
            bool is_server = false);

        /**
         * Handles a keepalive packet
         *
         * @param from
         * @param peer_address
         * @param packet
         * @param is_server
         */
        void handle_packet(
            const crypto_hash_t &from,
            const std::string &peer_address,
            const Types::Network::packet_keepalive_t &packet,
            bool is_server = false);

        /**
         * Handles a data packet
         *
         * @param from
         * @param peer_address
         * @param packet
         * @param is_server
         */
        void handle_packet(
            const crypto_hash_t &from,
            const std::string &peer_address,
            const Types::Network::packet_data_t &packet,
            bool is_server = false);

        /**
         * Replies via the server to a request by a client
         *
         * NOTE: message must be properly routed via the TO field
         *
         * @param message
         */
        void reply(const zmq_message_envelope_t &message);

        /**
         * The thread that sends keepalive messages
         */
        void send_keepalives();

        /**
         * The thread that sends peer exchange messages
         */
        void send_peer_exchanges();

        /**
         * The incoming message poller thread
         */
        void poller();

        std::atomic<bool> m_running, m_seed_mode;

        std::shared_ptr<PeerDB> m_peer_db;

        std::shared_ptr<Networking::ZMQServer> m_server;

        std::vector<std::shared_ptr<Networking::ZMQClient>> m_clients;

        std::set<crypto_hash_t> m_completed_handshake, m_clients_connected;

        std::thread m_poller_thread, m_keepalive_thread, m_peer_exchange_thread, m_connection_manager_thread;

        mutable std::mutex m_mutex_clients, m_mutex_handshake_completed;

        logger m_logger;

        std::condition_variable m_stopping;
    };
} // namespace P2P

#endif
