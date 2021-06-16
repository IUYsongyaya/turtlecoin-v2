// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "peer_database.h"

// static entry that we can use to lookup our own peer id in the database
static const auto PEER_ID_IDENTIFIER =
    crypto_hash_t("5440dd9b6683e3b2b0805eec3514ff3e23b7edea1bf29b434cd7a8447687650d");

namespace P2P
{
    PeerDB::PeerDB(const std::string &path)
    {
        m_env = Database::LMDB::getInstance(path);

        m_database = m_env->open_database("peerlist");

        auto info = m_env->open_database("local");

        const auto [error, value] = info->get<crypto_hash_t, crypto_hash_t>(PEER_ID_IDENTIFIER);

        if (!error)
        {
            m_peer_id = value;
        }
        else
        {
            m_peer_id = Crypto::random_hash();
        }

        info->put(PEER_ID_IDENTIFIER, m_peer_id);
    }

    Error PeerDB::add(const network_peer_t &entry)
    {
        if (entry.peer_id == m_peer_id)
        {
            return MAKE_ERROR_MSG(GENERIC_FAILURE, "We do not add ourselves to the peer list.");
        }

        const auto prune_time = (time(nullptr)) - Configuration::P2P::PEER_PRUNE_TIME;

        if (entry.last_seen < prune_time)
        {
            return MAKE_ERROR_MSG(GENERIC_FAILURE, "Peer last seen too far in the past.");
        }

        return m_database->put(entry.peer_id, entry.serialize());
    }

    size_t PeerDB::count() const
    {
        return m_database->count();
    }

    Error PeerDB::del(const network_peer_t &entry)
    {
        return m_database->del(entry.peer_id);
    }

    Error PeerDB::del(const crypto_hash_t &peer_id)
    {
        return m_database->del(peer_id);
    }

    bool PeerDB::exists(const crypto_hash_t &peer_id)
    {
        return m_database->exists(peer_id);
    }

    crypto_hash_t PeerDB::peer_id() const
    {
        return m_peer_id;
    }

    std::vector<crypto_hash_t> PeerDB::peer_ids() const
    {
        return m_database->list_keys<crypto_hash_t>();
    }

    std::vector<network_peer_t> PeerDB::peers() const
    {
        return m_database->get_all<crypto_hash_t, network_peer_t>();
    }

    Error PeerDB::prune()
    {
        const auto all_peers = peers();

        const auto prune_time = (time(nullptr)) - Configuration::P2P::PEER_PRUNE_TIME;

        for (const auto &peer : all_peers)
        {
            if (peer.last_seen < prune_time)
            {
                auto error = del(peer.hash());

                if (error)
                {
                    return error;
                }
            }
        }

        return MAKE_ERROR(SUCCESS);
    }

    Error PeerDB::touch(const crypto_hash_t &peer_id)
    {
        auto [error_get, peer] = m_database->get<crypto_hash_t, network_peer_t>(peer_id);

        if (error_get)
        {
            return error_get;
        }

        peer.last_seen = time(nullptr);

        return m_database->put(peer.peer_id, peer.serialize());
    }
} // namespace P2P
