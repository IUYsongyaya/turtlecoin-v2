// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CORE_BLOCKCHAIN_STORAGE_H
#define CORE_BLOCKCHAIN_STORAGE_H

#include <crypto.h>
#include <db_lmdb.h>
#include <types.h>

namespace Core
{
    class BlockchainStorage
    {
      public:
        /**
         * Create new instance of the blockchain storage in the specified path
         *
         * @param db_path
         */
        BlockchainStorage(const std::string &db_path);

        /**
         * Retrieves a singleton instance of the class
         *
         * @param db_path
         * @return
         */
        static std::shared_ptr<BlockchainStorage> getInstance(const std::string &db_path);

        /**
         * Checks whether the block with the given hash exists in the database
         *
         * @param block_hash
         * @return
         */
        bool block_exists(const crypto_hash_t &block_hash) const;

        /**
         * Checks whether the block with the given index exists in the database
         *
         * @param block_index
         * @return
         */
        bool block_exists(const uint64_t &block_index) const;

        /**
         * Retrieves the block and transactions within that block using the specified block hash
         *
         * @param block_hash
         * @return
         */
        std::tuple<Error, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
            get_block(const crypto_hash_t &block_hash) const;

        /**
         * Retrieves the block and transactions within that block using the specified block index
         *
         * @param block_height
         * @return
         */
        std::tuple<Error, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
            get_block(const uint64_t &block_index) const;

        /**
         * Retrieve the NEXT closest block hash by timestamp using the specified timestamp
         *
         * @param timestamp
         * @return
         */
        std::tuple<Error, uint64_t, crypto_hash_t> get_block_by_timestamp(const uint64_t &timestamp) const;

        /**
         * Retrieve the total number of blocks stored in the database
         *
         * @return
         */
        size_t get_block_count() const;

        /**
         * Retrieve the block hash for the given block index
         *
         * @param block_index
         * @return
         */
        std::tuple<Error, crypto_hash_t> get_block_hash(const uint64_t &block_index) const;

        /**
         * Retrieve the block hash for the given block hash
         *
         * @param block_hash
         * @return
         */
        std::tuple<Error, uint64_t> get_block_index(const crypto_hash_t &block_hash) const;

        /**
         * Retrieves the maximum transaction output global index from the database
         *
         * @return
         */
        std::tuple<Error, uint64_t> get_maximum_global_index() const;

        /**
         * Retrieves the transaction output for the specified global index
         *
         * @param global_index
         * @return
         */
        std::tuple<Error, Types::Blockchain::transaction_output_t>
            get_output_by_global_index(const uint64_t &global_index) const;

        /**
         * Retrieve the transaction outputs for the specified global indexes
         *
         * @param global_indexes
         * @return
         */
        std::tuple<Error, std::map<uint64_t, Types::Blockchain::transaction_output_t>>
            get_outputs_by_global_indexes(const std::vector<uint64_t> &global_indexes) const;

        /**
         * Retrieves the transaction with the specified hash
         *
         * @param txn_hash
         * @return
         */
        std::tuple<Error, Types::Blockchain::transaction_t, crypto_hash_t>
            get_transaction(const crypto_hash_t &txn_hash) const;

        /**
         * Retrieves the global indexes for the transaction with the specified hash
         *
         * @param txn_hash
         * @return
         */
        std::tuple<Error, std::vector<uint64_t>> get_transaction_indexes(const crypto_hash_t &txn_hash) const;

        /**
         * Checks if the specified key image exists in the database
         *
         * @param key_image
         * @return
         */
        bool key_image_exists(const crypto_key_image_t &key_image) const;

        /**
         * Checks if the specified key image exists in the database
         *
         * @param key_images
         * @return
         */
        std::map<crypto_key_image_t, bool> key_image_exists(const std::vector<crypto_key_image_t> &key_images) const;

        /**
         * Saves the block with the transactions specified in the database
         *
         * @param block
         * @param transactions
         * @return
         */
        Error put_block(
            const Types::Blockchain::block_t &block,
            const std::vector<Types::Blockchain::transaction_t> &transactions);

      private:
        /**
         * Saves the specified key image to the database
         *
         * @param db_tx
         * @param key_image
         * @return
         */
        Error put_key_image(std::unique_ptr<Database::LMDBTransaction> &db_tx, const crypto_key_image_t &key_image);

        /**
         * Saves the specified transaction to the database
         *
         * @param db_tx
         * @param transaction
         * @return
         */
        std::tuple<Error, crypto_hash_t> put_transaction(
            std::unique_ptr<Database::LMDBTransaction> &db_tx,
            const Types::Blockchain::transaction_t &transaction);

        /**
         * Saves the specified block hash for the specified transaction hash
         *
         * @param db_tx
         * @param txn_hash
         * @param block_hash
         * @return
         */
        Error put_transaction_block_hash(
            std::unique_ptr<Database::LMDBTransaction> &db_tx,
            const crypto_hash_t &txn_hash,
            const crypto_hash_t &block_hash);

        /**
         * Saves the specified transaction indexes to the database
         *
         * @param db_tx
         * @param tx_hash
         * @param indexes
         * @return
         */
        Error put_transaction_indexes(
            std::unique_ptr<Database::LMDBTransaction> &db_tx,
            const crypto_hash_t &tx_hash,
            const std::vector<uint64_t> &indexes);

        /**
         * Saves the specified transaction output to the database
         *
         * @param db_tx
         * @param index
         * @param output
         * @return
         */
        std::tuple<Error, uint64_t> put_transaction_output(
            std::unique_ptr<Database::LMDBTransaction> &db_tx,
            const uint64_t &index,
            const Types::Blockchain::transaction_output_t &output);

        std::shared_ptr<Database::LMDB> m_db_env;

        std::shared_ptr<Database::LMDBDatabase> m_blocks, m_block_indexes, m_block_timestamps, m_transactions,
            m_key_images, m_global_indexes, m_transaction_indexes, m_transaction_block_hashes;

        std::mutex write_mutex;
    };
} // namespace Core

#endif // CORE_BLOCKCHAIN_STORAGE_H
