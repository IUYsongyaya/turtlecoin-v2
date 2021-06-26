// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "network_node.h"

using namespace BaseTypes;
using namespace Types::Network;

namespace P2P
{
    NetworkNode::NetworkNode(logger &logger, const std::string &path, const uint16_t &bind_port):
        m_running(false), m_logger(logger)
    {
        m_peer_db = std::make_shared<PeerDB>(path);

        m_peer_db->prune();

        m_server = std::make_shared<Networking::ZMQServer>(m_logger, bind_port);
    }

    NetworkNode::~NetworkNode()
    {
        m_running = false;

        m_server.reset();

        if (m_poller_thread.joinable())
        {
            m_poller_thread.join();
        }

        if (m_keepalive_thread.joinable())
        {
            m_keepalive_thread.join();
        }

        if (m_peer_exchange_thread.joinable())
        {
            m_peer_exchange_thread.join();
        }

        if (m_connection_manager_thread.joinable())
        {
            m_connection_manager_thread.join();
        }
    }

    packet_handshake_t NetworkNode::build_handshake() const
    {
        packet_handshake_t packet(m_peer_db->peer_id(), m_server->port());

        packet.peers = build_peer_list();

        return packet;
    }

    std::vector<network_peer_t> NetworkNode::build_peer_list() const
    {
        std::vector<network_peer_t> results = m_peer_db->peers();

        if (results.size() > Configuration::P2P::MAXIMUM_PEERS_EXCHANGED)
        {
            results.resize(Configuration::P2P::MAXIMUM_PEERS_EXCHANGED);
        }

        return results;
    }

    Error NetworkNode::connect(const std::string &unsafe_host, const uint16_t &port)
    {
        std::scoped_lock lock(m_mutex_clients);

        const auto host = Networking::zmq_sanitize_host(unsafe_host);

        const auto hash = Networking::zmq_host_port_hash(host, port);

        m_logger->debug("Attempting connection to: {}:{} => {}", host, port, hash.to_string());

        if (m_clients_connected.find(hash) != m_clients_connected.end())
        {
            return MAKE_ERROR_MSG(GENERIC_FAILURE, "Already connected to specified host and port");
        }

        auto client = std::make_shared<Networking::ZMQClient>(m_logger);

        auto error = client->connect(host, port);

        if (error)
        {
            return error;
        }

        m_clients_connected.insert(hash);

        const auto packet = build_handshake();

        auto message = zmq_message_envelope_t(packet);

        client->send(message);

        m_clients.push_back(client);

        return MAKE_ERROR(SUCCESS);
    }

    void NetworkNode::connection_manager()
    {
        while (m_running)
        {
            const auto delta_connections = Configuration::P2P::DEFAULT_CONNECTION_COUNT - outgoing_connections();

            if (delta_connections > 0)
            {
                const auto peers = m_peer_db->peers(delta_connections);

                if (!peers.empty())
                {
                    for (const auto &peer : peers)
                    {
                        // do not connect to ourselves
                        if (peer.peer_id == m_peer_db->peer_id())
                        {
                            continue;
                        }

                        auto error = connect(peer.address.to_string(), peer.port);

                        if (error)
                        {
                            m_logger->debug("Error connecting to peer: {}", error.to_string());
                        }
                    }
                }
            }

            THREAD_SLEEP_MS(Configuration::P2P::CONNECTION_MANAGER_INTERVAL);
        }
    }

    void NetworkNode::handle_incoming_message(const zmq_message_envelope_t &message, bool is_server)
    {
        try
        {
            auto reader = deserializer_t(message.payload);

            const auto type = reader.varint<uint64_t>(true);

            switch (type)
            {
                case NetworkPacketTypes::NETWORK_HANDSHAKE:
                    return handle_packet(message.from, message.peer_address, packet_handshake_t(reader), is_server);
                case NetworkPacketTypes::NETWORK_PEER_EXCHANGE:
                    return handle_packet(message.from, message.peer_address, packet_peer_exchange_t(reader), is_server);
                case NetworkPacketTypes::NETWORK_KEEPALIVE:
                    return handle_packet(message.from, message.peer_address, packet_keepalive_t(reader), is_server);
                case NetworkPacketTypes::NETWORK_DATA:
                    return handle_packet(message.from, message.peer_address, packet_data_t(reader), is_server);
                default:
                    throw std::runtime_error("Unknown packet type detected");
            }
        }
        catch (const std::exception &e)
        {
            // TODO: if we cannot handle the message, we need to disconnect whoever sent it SOMEHOW
            m_logger->debug("Could not handle incoming P2P message: {}", e.what());
        }
    }

    void NetworkNode::handle_packet(
        const crypto_hash_t &from,
        const std::string &peer_address,
        const Types::Network::packet_handshake_t &packet,
        bool is_server)
    {
        if (is_server && m_completed_handshake.find(from) != m_completed_handshake.end())
        {
            throw std::runtime_error("Handshake already completed, protocol violation.");
        }

        // we don't talk to ourselves
        if (from == m_server->identity() || packet.peer_id == m_peer_db->peer_id())
        {
            return;
        }

        // we don't talk to peers that are not speaking at least the minimum version
        if (packet.version < Configuration::P2P::MINIMUM_VERSION)
        {
            return;
        }

        if (packet.peers.size() > Configuration::P2P::MAXIMUM_PEERS_EXCHANGED)
        {
            throw std::runtime_error(
                "Handshake contains more than the maximum number of peers accepted, protocol violation.");
        }

        std::cout << is_server << "\t" << packet << std::endl;

        {
            network_peer_t peer(ip_address_t(peer_address), packet.peer_id, packet.peer_port);

            m_peer_db->add(peer);
        }

        for (const auto &peer : packet.peers)
        {
            if (peer.peer_id == packet.peer_id)
            {
                continue;
            }

            m_peer_db->add(peer);
        }

        if (is_server)
        {
            const auto reply_handshake = build_handshake();

            zmq_message_envelope_t message(from, reply_handshake);

            reply(message);

            std::scoped_lock lock(m_mutex_handshake_completed);

            m_completed_handshake.insert(from);
        }
    }

    void NetworkNode::handle_packet(
        const crypto_hash_t &from,
        const std::string &peer_address,
        const Types::Network::packet_data_t &packet,
        bool is_server)
    {
        if (m_completed_handshake.find(from) == m_completed_handshake.end())
        {
            throw std::runtime_error("Handshake not completed first, protocol violation.");
        }

        // we don't talk to ourselves
        if (from == m_server->identity())
        {
            return;
        }

        // we don't talk to peers that are not speaking at least the minimum version
        if (packet.version < Configuration::P2P::MINIMUM_VERSION)
        {
            return;
        }

        std::cout << is_server << "\t" << from << std::endl << packet << std::endl;
    }

    void NetworkNode::handle_packet(
        const crypto_hash_t &from,
        const std::string &peer_address,
        const Types::Network::packet_keepalive_t &packet,
        bool is_server)
    {
        if (!is_server)
        {
            m_peer_db->touch(packet.peer_id);

            return;
        }

        if (m_completed_handshake.find(from) == m_completed_handshake.end())
        {
            throw std::runtime_error("Handshake not completed first, protocol violation.");
        }

        // we don't talk to ourselves
        if (from == m_server->identity() || packet.peer_id == m_peer_db->peer_id())
        {
            return;
        }

        // we don't talk to peers that are not speaking at least the minimum version
        if (packet.version < Configuration::P2P::MINIMUM_VERSION)
        {
            return;
        }

        std::cout << is_server << "\t" << packet << std::endl;

        packet_keepalive_t reply_keepalive(m_peer_db->peer_id());

        zmq_message_envelope_t message(from, reply_keepalive);

        reply(message);

        m_peer_db->touch(packet.peer_id);
    }

    void NetworkNode::handle_packet(
        const crypto_hash_t &from,
        const std::string &peer_address,
        const Types::Network::packet_peer_exchange_t &packet,
        bool is_server)
    {
        if (is_server && m_completed_handshake.find(from) == m_completed_handshake.end())
        {
            throw std::runtime_error("Handshake not completed first, protocol violation.");
        }

        // we don't talk to ourselves
        if (from == m_server->identity() || packet.peer_id == m_peer_db->peer_id())
        {
            return;
        }

        // we don't talk to peers that are not speaking at least the minimum version
        if (packet.version < Configuration::P2P::MINIMUM_VERSION)
        {
            return;
        }

        std::cout << is_server << "\t" << packet << std::endl;

        {
            network_peer_t peer(ip_address_t(peer_address), packet.peer_id, packet.peer_port);

            m_peer_db->add(peer);
        }

        for (const auto &peer : packet.peers)
        {
            if (peer.peer_id == packet.peer_id)
            {
                continue;
            }

            m_peer_db->add(peer);
        }

        if (is_server)
        {
            packet_peer_exchange_t reply_peer_exchange(m_peer_db->peer_id(), m_server->port());

            reply_peer_exchange.peers = build_peer_list();

            zmq_message_envelope_t message(from, reply_peer_exchange);

            reply(message);
        }
    }

    size_t NetworkNode::incoming_connections() const
    {
        return m_server->connections();
    }

    size_t NetworkNode::outgoing_connections() const
    {
        std::scoped_lock lock(m_mutex_clients);

        return m_clients.size();
    }

    crypto_hash_t NetworkNode::peer_id() const
    {
        return m_peer_db->peer_id();
    }

    std::shared_ptr<PeerDB> NetworkNode::peers() const
    {
        return m_peer_db;
    }

    void NetworkNode::poller()
    {
        while (m_running)
        {
            if (!m_server->messages().empty())
            {
                const auto message = m_server->messages().pop();

                handle_incoming_message(message, true);
            }

            std::unique_lock lock(m_mutex_clients);

            const auto clients = m_clients;

            lock.unlock();

            for (const auto &client : clients)
            {
                if (!client->messages().empty())
                {
                    const auto message = client->messages().pop();

                    handle_incoming_message(message);
                }
            }

            THREAD_SLEEP();
        }
    }

    void NetworkNode::reply(const zmq_message_envelope_t &message)
    {
        m_server->send(message);
    }

    bool NetworkNode::running() const
    {
        return m_running;
    }

    void NetworkNode::send(const zmq_message_envelope_t &message)
    {
        std::scoped_lock lock(m_mutex_clients);

        for (const auto &client : m_clients)
        {
            client->send(message);
        }
    }

    void NetworkNode::send_keepalives()
    {
        while (m_running)
        {
            THREAD_SLEEP_MS(Configuration::P2P::KEEPALIVE_INTERVAL);

            packet_keepalive_t packet(m_peer_db->peer_id());

            send(packet);
        }
    }

    void NetworkNode::send_peer_exchanges()
    {
        while (m_running)
        {
            THREAD_SLEEP_MS(Configuration::P2P::PEER_EXCHANGE_INTERVAL);

            packet_peer_exchange_t packet(m_peer_db->peer_id(), m_server->port());

            send(packet);
        }
    }

    Error NetworkNode::start()
    {
        if (!m_running)
        {
            {
                auto error = m_server->bind();

                if (error)
                {
                    return error;
                }
            }

            m_running = true;

            m_poller_thread = std::thread(&NetworkNode::poller, this);

            bool connected_to_seed = false;

            for (const auto &seed_node : Configuration::P2P::SEED_NODES)
            {
                auto error = connect(seed_node.host, seed_node.port);

                if (!error)
                {
                    connected_to_seed = true;
                }
            }

            /**
             * If we cannot connect to ANY seed node and our peer list database is empty
             * then we need to fail out as we cannot connect to the peer to peer network
             */
            if (!connected_to_seed && m_peer_db->count() == 0)
            {
                m_running = false;

                if (m_poller_thread.joinable())
                {
                    m_poller_thread.join();
                }

                return MAKE_ERROR_MSG(GENERIC_FAILURE, "Could not connect to any seed nodes.");
            }

            m_keepalive_thread = std::thread(&NetworkNode::send_keepalives, this);

            m_peer_exchange_thread = std::thread(&NetworkNode::send_peer_exchanges, this);

            m_connection_manager_thread = std::thread(&NetworkNode::connection_manager, this);
        }

        return MAKE_ERROR(SUCCESS);
    }
} // namespace P2P
