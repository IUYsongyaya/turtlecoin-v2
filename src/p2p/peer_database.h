// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_P2P_DATABASE_H
#define TURTLECOIN_P2P_DATABASE_H

#include <crypto.h>
#include <db_lmdb.h>
#include <types.h>

using namespace Types::Network;

namespace P2P
{
    class PeerDB
    {
      public:
        PeerDB(const std::string &path);

        Error add(const network_peer_t &entry);

        size_t count() const;

        Error del(const network_peer_t &entry);

        Error del(const crypto_hash_t &peer_id);

        bool exists(const crypto_hash_t &peer_id);

        crypto_hash_t peer_id() const;

        std::vector<crypto_hash_t> peer_ids() const;

        std::vector<network_peer_t> peers() const;

        Error prune();

        Error touch(const crypto_hash_t &peer_id);

      private:
        std::shared_ptr<Database::LMDB> m_env;

        std::shared_ptr<Database::LMDBDatabase> m_database;

        crypto_hash_t m_peer_id;
    };
} // namespace P2P

#endif
