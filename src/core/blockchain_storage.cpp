// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "blockchain_storage.h"

static std::shared_ptr<Core::BlockchainStorage> blockchain_storage_instance;

namespace Core
{
    BlockchainStorage::BlockchainStorage(const std::string &db_path)
    {
        m_db_env = Database::LMDB::getInstance(db_path);

        m_blocks = m_db_env->open_database("blocks");

        m_block_heights = m_db_env->open_database("block_heights");

        m_transactions = m_db_env->open_database("transactions");

        m_key_images = m_db_env->open_database("key_images");

        m_global_indexes = m_db_env->open_database("global_indexes");
    }

    std::shared_ptr<BlockchainStorage> BlockchainStorage::getInstance(const std::string &db_path)
    {
        if (!blockchain_storage_instance)
        {
            blockchain_storage_instance = std::make_shared<BlockchainStorage>(db_path);
        }

        return blockchain_storage_instance;
    }

    bool BlockchainStorage::check_key_image_spent(const crypto_key_image_t &key_image) const
    {
        return m_key_images->exists(key_image);
    }

    std::map<crypto_key_image_t, bool>
        BlockchainStorage::check_key_image_spent(const std::vector<crypto_key_image_t> &key_images) const
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

    bool BlockchainStorage::block_exists(const crypto_hash_t &block_hash) const
    {
        return m_blocks->exists(block_hash);
    }

    bool BlockchainStorage::block_exists(const uint64_t &block_height) const
    {
        const auto [error, block_hash] = m_block_heights->get<crypto_hash_t>(block_height);

        return !error;
    }

    std::tuple<Error, Types::Blockchain::transaction_t>
        BlockchainStorage::get_transaction(const crypto_hash_t &txn_hash) const
    {
        const auto [error, txn_data] = m_transactions->get(txn_hash);

        if (error)
        {
            return {TRANSACTION_NOT_FOUND, {}};
        }

        deserializer_t reader(txn_data);

        const auto type = reader.varint<uint64_t>(true);

        switch (type)
        {
            case Types::Blockchain::TransactionType::GENESIS:
                return {SUCCESS, Types::Blockchain::genesis_transaction_t(reader)};
            case Types::Blockchain::TransactionType::STAKER_REWARD:
                return {SUCCESS, Types::Blockchain::staker_reward_transaction_t(reader)};
            case Types::Blockchain::TransactionType::NORMAL:
                return {SUCCESS, Types::Blockchain::committed_normal_transaction_t(reader)};
            case Types::Blockchain::TransactionType::STAKE:
                return {SUCCESS, Types::Blockchain::committed_stake_transaction_t(reader)};
            case Types::Blockchain::TransactionType::RECALL_STAKE:
                return {SUCCESS, Types::Blockchain::committed_recall_stake_transaction_t(reader)};
            case Types::Blockchain::TransactionType::STAKE_REFUND:
                return {SUCCESS, Types::Blockchain::stake_refund_transaction_t(reader)};
            default:
                return {UNKNOWN_TRANSACTION_TYPE, {}};
        }
    }

    std::tuple<Error, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
        BlockchainStorage::get_block(const crypto_hash_t &block_hash) const
    {
        const auto [error, block] = m_blocks->get<crypto_hash_t, Types::Blockchain::block_t>(block_hash);

        if (error)
        {
            return {BLOCK_NOT_FOUND, {}, {}};
        }

        std::vector<Types::Blockchain::transaction_t> transactions;

        for (const auto &txn : block.transactions)
        {
            const auto [txn_error, transaction] = get_transaction(txn);

            if (txn_error)
            {
                return {TRANSACTION_NOT_FOUND, {}, {}};
            }

            transactions.push_back(transaction);
        }

        return {SUCCESS, block, transactions};
    }

    std::tuple<Error, Types::Blockchain::block_t, std::vector<Types::Blockchain::transaction_t>>
        BlockchainStorage::get_block(const uint64_t &block_height) const
    {
        const auto [error, block_hash] = m_block_heights->get<crypto_hash_t>(block_height);

        if (error)
        {
            return {BLOCK_NOT_FOUND, {}, {}};
        }

        return get_block(block_hash);
    }

    size_t BlockchainStorage::get_maximum_global_index() const
    {
        const auto count = m_global_indexes->count();

        return (count == 0) ? count : count - 1;
    }

    std::tuple<Error, Types::Blockchain::transaction_output_t>
        BlockchainStorage::get_output_by_global_index(size_t global_index) const
    {
        if (global_index > get_maximum_global_index())
        {
            return {GLOBAL_INDEX_OUT_OF_BOUNDS, {}};
        }

        return m_global_indexes->get<Types::Blockchain::transaction_output_t>(global_index);
    }

    Error BlockchainStorage::put_block(
        const Types::Blockchain::block_t &block,
        const std::vector<Types::Blockchain::transaction_t> &transactions)
    {
        /**
         * Sanity check transaction order before write
         */
        {
            std::vector<crypto_hash_t> block_tx_hashes, txn_hashes;

            for (const auto &tx : block.transactions)
            {
                block_tx_hashes.push_back(tx);
            }

            for (const auto &tx : transactions)
            {
                std::visit([&txn_hashes](auto &&arg) { txn_hashes.push_back(arg.hash()); }, tx);
            }

            const auto block_hashes = Crypto::Hashing::sha3(block_tx_hashes);

            const auto tx_hashes = Crypto::Hashing::sha3(txn_hashes);

            if (block_hashes != tx_hashes)
            {
                return BLOCK_TXN_ORDER;
            }
        }

        std::scoped_lock lock(write_mutex);

        auto db_tx = m_db_env->transaction();

        {
            auto error =
                std::visit([this, &db_tx](auto &&arg) { return put_transaction(db_tx, arg); }, block.reward_tx);

            if (error)
            {
                return error;
            }
        }

        for (const auto &transaction : transactions)
        {
            auto error = put_transaction(db_tx, transaction);

            if (error)
            {
                return error;
            }
        }

        const auto block_hash = block.hash();

        {
            db_tx->set_database(m_blocks);

            auto error = db_tx->put(block_hash, block.serialize());

            if (error)
            {
                return error;
            }
        }

        {
            db_tx->set_database(m_block_heights);

            auto error = db_tx->put(block.block_index, block_hash);

            if (error)
            {
                return error;
            }
        }

        return db_tx->commit();
    }

    Error BlockchainStorage::put_transaction(
        std::unique_ptr<Database::LMDBTransaction> &db_tx,
        const Types::Blockchain::transaction_t &transaction)
    {
        db_tx->set_database(m_transactions);

        {
            auto error = std::visit(
                [this, &db_tx](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    {
                        auto error = db_tx->put(arg.hash(), arg.serialize());

                        if (error)
                        {
                            return error;
                        }
                    }

                    if constexpr (
                        std::is_same_v<
                            T,
                            Types::Blockchain::
                                committed_normal_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_stake_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_recall_stake_transaction_t>)
                    {
                        for (const auto &key_image : arg.key_images)
                        {
                            auto error = put_key_image(db_tx, key_image);

                            if (error)
                            {
                                return error;
                            }
                        }
                    }

                    return Error(SUCCESS);
                },
                transaction);

            if (error)
            {
                return error;
            }
        }

        uint64_t count = m_global_indexes->count();

        db_tx->set_database(m_global_indexes);

        {
            auto error = std::visit(
                [&count, &db_tx](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    if constexpr (
                        std::is_same_v<
                            T,
                            Types::Blockchain::
                                committed_normal_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_stake_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_recall_stake_transaction_t> || std::is_same_v<T, Types::Blockchain::genesis_transaction_t>)
                    {
                        for (const auto &output : arg.outputs)
                        {
                            auto error = db_tx->put(count, output.serialize_output());

                            if (error)
                            {
                                return error;
                            }

                            count++;
                        }
                    }
                    else if constexpr (std::is_same_v<T, Types::Blockchain::stake_refund_transaction_t>)
                    {
                        /**
                         * We set the amount to 0 here as a) the amount is masked anyways and b) it does not
                         * matter for generating or checking signatures
                         */
                        const auto output =
                            Types::Blockchain::transaction_output_t(arg.public_ephemeral, 0, arg.commitment);

                        auto error = db_tx->put(count, output.serialize_output());

                        if (error)
                        {
                            return error;
                        }

                        count++;
                    }

                    return Error(SUCCESS);
                },
                transaction);

            if (error)
            {
                return error;
            }
        }

        return SUCCESS;
    }

    Error BlockchainStorage::put_key_image(
        std::unique_ptr<Database::LMDBTransaction> &db_tx,
        const crypto_key_image_t &key_image)
    {
        db_tx->set_database(m_key_images);

        return db_tx->put<crypto_key_image_t, std::vector<uint8_t>>(key_image, {});
    }
} // namespace Core
