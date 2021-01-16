// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "blockchain_storage.h"

static std::shared_ptr<TurtleCoin::Core::BlockchainStorage> blockchain_storage_instance;

namespace TurtleCoin::Core
{
    BlockchainStorage::BlockchainStorage(const std::string &db_path)
    {
        m_db_env = Database::LMDB::getInstance(db_path);

        m_blocks = m_db_env->open_database("blocks");

        m_block_heights = m_db_env->open_database("block_heights");

        m_transactions = m_db_env->open_database("transactions");

        m_key_images = m_db_env->open_database("key_images");
    }

    std::shared_ptr<BlockchainStorage> BlockchainStorage::getInstance(const std::string &db_path)
    {
        if (!blockchain_storage_instance)
        {
            blockchain_storage_instance = std::make_shared<BlockchainStorage>(db_path);
        }

        return blockchain_storage_instance;
    }

    bool BlockchainStorage::check_key_image_spent(const crypto_key_image_t &key_image)
    {
        return m_key_images->exists(key_image);
    }

    std::map<crypto_key_image_t, bool>
        BlockchainStorage::check_key_image_spent(const std::vector<crypto_key_image_t> &key_images)
    {
        auto txn = m_key_images->transaction(true);

        std::map<crypto_key_image_t, bool> results;

        for (const auto &key_image : key_images)
        {
            const auto exists = txn->exists(key_image);

            results.insert({key_image, exists});
        }

        return results;
    }

    std::tuple<bool, Types::Blockchain::transaction_t> BlockchainStorage::get_transaction(const crypto_hash_t &txn_hash)
    {
        const auto [found, txn_data] = m_transactions->get(txn_hash);

        if (!found)
        {
            return {false, {}};
        }

        deserializer_t reader(txn_data);

        const auto type = reader.varint<uint64_t>(true);

        switch (type)
        {
            case Configuration::Transaction::Types::COINBASE:
                return {true, Types::Blockchain::coinbase_transaction_t(reader)};
            case Configuration::Transaction::Types::NORMAL:
                return {true, Types::Blockchain::committed_normal_transaction_t(reader)};
            case Configuration::Transaction::Types::STAKE:
                return {true, Types::Blockchain::committed_stake_transaction_t(reader)};
            case Configuration::Transaction::Types::RECALL_STAKE:
                return {true, Types::Blockchain::committed_recall_stake_transaction_t(reader)};
            case Configuration::Transaction::Types::STAKE_REFUND:
                return {true, Types::Blockchain::stake_refund_transaction_t(reader)};
            default:
                return {false, {}};
        }
    }

    std::tuple<bool, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
        BlockchainStorage::get_block(const crypto_hash_t &block_hash)
    {
        const auto [block_found, block] = m_blocks->get<crypto_hash_t, Types::Blockchain::block_t>(block_hash);

        if (!block_found)
        {
            return {false, {}, {}};
        }

        std::vector<Types::Blockchain::transaction_t> transactions;

        for (const auto &txn : block.transactions)
        {
            const auto [txn_found, transaction] = get_transaction(txn);

            if (!txn_found)
            {
                // TODO: Log error as if we have the block, we should have the transaction
                return {false, {}, {}};
            }

            transactions.push_back(transaction);
        }

        return {true, block, transactions};
    }

    std::tuple<bool, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
        BlockchainStorage::get_block(const uint64_t &block_height)
    {
        const auto [found, block_hash] = m_block_heights->get<crypto_hash_t>(block_height);

        if (!found)
        {
            return {false, {}, {}};
        }

        return get_block(block_hash);
    }

    bool BlockchainStorage::mark_key_image_spent(const crypto_key_image_t &key_image)
    {
        return m_key_images->put<crypto_key_image_t, std::vector<uint8_t>>(key_image, {});
    }

    bool BlockchainStorage::mark_key_image_spent(const std::vector<crypto_key_image_t> &key_images)
    {
        return m_key_images->put<crypto_key_image_t, std::vector<uint8_t>>(key_images, {});
    }
} // namespace TurtleCoin::Core
