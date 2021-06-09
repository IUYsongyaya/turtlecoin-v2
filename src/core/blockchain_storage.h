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

        static std::shared_ptr<BlockchainStorage> getInstance(const std::string &db_path);

        std::tuple<bool, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
            get_block(const crypto_hash_t &block_hash);

        std::tuple<bool, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
            get_block(const uint64_t &block_height);

        bool put_block(
            const Types::Blockchain::block_t &block,
            const std::vector<Types::Blockchain::transaction_t> &transactions);

        std::tuple<bool, Types::Blockchain::transaction_t> get_transaction(const crypto_hash_t &txn_hash);

        bool check_key_image_spent(const crypto_key_image_t &key_image);

        std::map<crypto_key_image_t, bool> check_key_image_spent(const std::vector<crypto_key_image_t> &key_images);

      private:
        bool mark_key_image_spent(const crypto_key_image_t &key_image);

        bool mark_key_image_spent(const std::vector<crypto_key_image_t> &key_images);

        template<typename T>
        bool put_transaction(std::unique_ptr<Database::LMDBTransaction> &db_tx, const T &transaction)
        {
            return db_tx->put(transaction.hash(), transaction.serialize()) == 0;
        }

        std::shared_ptr<Database::LMDB> m_db_env;

        std::shared_ptr<Database::LMDBDatabase> m_blocks, m_block_heights, m_transactions, m_key_images;

        std::mutex blocks_mutex;
    };
} // namespace TurtleCoin::Core

#endif // CORE_BLOCKCHAIN_STORAGE_H
