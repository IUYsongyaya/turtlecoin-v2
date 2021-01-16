// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CORE_BLOCKCHAIN_STORAGE_H
#define CORE_BLOCKCHAIN_STORAGE_H

#include <crypto.h>
#include <db_lmdb.h>
#include <types.h>

namespace TurtleCoin::Core
{
    class BlockchainStorage
    {
      public:
        BlockchainStorage(const std::string &db_path);

        std::shared_ptr<BlockchainStorage> getInstance(const std::string &db_path);

        std::tuple<bool, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
            get_block(const crypto_hash_t &block_hash);

        std::tuple<bool, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
            get_block(const uint64_t &block_height);

        std::tuple<bool, Types::Blockchain::transaction_t> get_transaction(const crypto_hash_t &txn_hash);

        bool check_key_image_spent(const crypto_key_image_t &key_image);

        std::map<crypto_key_image_t, bool> check_key_image_spent(const std::vector<crypto_key_image_t> &key_images);

        bool mark_key_image_spent(const crypto_key_image_t &key_image);

        bool mark_key_image_spent(const std::vector<crypto_key_image_t> &key_images);

      private:
        std::shared_ptr<Database::LMDB> m_db_env;

        std::shared_ptr<Database::LMDBDatabase> m_blocks, m_block_heights, m_transactions, m_key_images;
    };
} // namespace TurtleCoin::Core

#endif // CORE_BLOCKCHAIN_STORAGE_H
