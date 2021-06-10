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

        m_block_indexes = m_db_env->open_database("block_indexes");

        m_transactions = m_db_env->open_database("transactions");

        m_key_images = m_db_env->open_database("key_images");

        m_global_indexes = m_db_env->open_database("global_indexes");

        m_transaction_indexes = m_db_env->open_database("transaction_indexes");
    }

    std::shared_ptr<BlockchainStorage> BlockchainStorage::getInstance(const std::string &db_path)
    {
        if (!blockchain_storage_instance)
        {
            blockchain_storage_instance = std::make_shared<BlockchainStorage>(db_path);
        }

        return blockchain_storage_instance;
    }

    bool BlockchainStorage::block_exists(const crypto_hash_t &block_hash) const
    {
        return m_blocks->exists(block_hash);
    }

    bool BlockchainStorage::block_exists(const uint64_t &block_index) const
    {
        return m_block_indexes->exists(block_index);
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

        // loop through the transactions in the block and retrieve them
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
        BlockchainStorage::get_block(const uint64_t &block_index) const
    {
        const auto [error, block_hash] = m_block_indexes->get<crypto_hash_t>(block_index);

        if (error)
        {
            return {BLOCK_NOT_FOUND, {}, {}};
        }

        return get_block(block_hash);
    }

    uint64_t BlockchainStorage::get_maximum_global_index() const
    {
        const auto count = m_global_indexes->count();

        // the result is -1 as count is inclusive of 0
        return (count == 0) ? count : count - 1;
    }

    std::tuple<Error, Types::Blockchain::transaction_output_t>
        BlockchainStorage::get_output_by_global_index(const uint64_t &global_index) const
    {
        if (global_index > get_maximum_global_index())
        {
            return {GLOBAL_INDEX_OUT_OF_BOUNDS, {}};
        }

        return m_global_indexes->get<Types::Blockchain::transaction_output_t>(global_index);
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

        // figure out what type of transaction it is
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

    std::tuple<Error, std::vector<uint64_t>>
        BlockchainStorage::get_transaction_indexes(const crypto_hash_t &txn_hash) const
    {
        const auto [error, data] = m_transaction_indexes->get(txn_hash);

        if (error)
        {
            return {TRANSACTION_NOT_FOUND, {}};
        }

        std::vector<uint64_t> result;

        deserializer_t reader(data);

        // the indexes are stored in a bytestream to save space so we need to read until finished
        while (reader.unread_bytes() > 0)
        {
            try
            {
                result.push_back(reader.varint<uint64_t>());
            }
            catch (...)
            {
                return {DESERIALIZATION_ERROR, {}};
            }
        }

        return {SUCCESS, result};
    }

    bool BlockchainStorage::key_image_exists(const crypto_key_image_t &key_image) const
    {
        return m_key_images->exists(key_image);
    }

    std::map<crypto_key_image_t, bool>
        BlockchainStorage::key_image_exists(const std::vector<crypto_key_image_t> &key_images) const
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

    Error BlockchainStorage::put_block(
        const Types::Blockchain::block_t &block,
        const std::vector<Types::Blockchain::transaction_t> &transactions)
    {
        /**
         * Sanity check transaction order before write
         */
        {
            // verify that the number of transactions match what is expected
            if (transactions.size() != block.transactions.size())
            {
                return BLOCK_TRANSACTIONS_MISMATCH;
            }

            // dump the transaction hashes into two vectors so that we can easily compare them
            std::vector<crypto_hash_t> block_tx_hashes, txn_hashes;

            for (const auto &tx : block.transactions)
            {
                block_tx_hashes.push_back(tx);
            }

            for (const auto &tx : transactions)
            {
                std::visit([&txn_hashes](auto &&arg) { txn_hashes.push_back(arg.hash()); }, tx);
            }

            // hash the vectors to get a result that we can match against
            const auto block_hashes = Crypto::Hashing::sha3(block_tx_hashes);

            const auto tx_hashes = Crypto::Hashing::sha3(txn_hashes);

            /**
             * Compare the resulting hashes and if they do not match, then kick back out.
             * The reason that we do this is that it guarantees that the order of the transactions
             * are processed in is the same at each node so that the global indexes match across
             * multiple nodes
             */
            if (block_hashes != tx_hashes)
            {
                return BLOCK_TXN_ORDER;
            }
        }

        std::scoped_lock lock(write_mutex);

        auto db_tx = m_db_env->transaction();

        // Push the block reward transaction into the database
        {
            auto error =
                std::visit([this, &db_tx](auto &&arg) { return put_transaction(db_tx, arg); }, block.reward_tx);

            if (error)
            {
                return error;
            }
        }

        // loop through the individual transactions in the block and push them into the database
        for (const auto &transaction : transactions)
        {
            auto error = put_transaction(db_tx, transaction);

            if (error)
            {
                return error;
            }
        }

        const auto block_hash = block.hash();

        // push the block itself into the database
        {
            db_tx->set_database(m_blocks);

            auto error = db_tx->put(block_hash, block.serialize());

            if (error)
            {
                return error;
            }
        }

        // push the block index into the database for easy retrieval later
        {
            db_tx->set_database(m_block_indexes);

            auto error = db_tx->put(block.block_index, block_hash);

            if (error)
            {
                return error;
            }
        }

        return db_tx->commit();
    }

    Error BlockchainStorage::put_key_image(
        std::unique_ptr<Database::LMDBTransaction> &db_tx,
        const crypto_key_image_t &key_image)
    {
        db_tx->set_database(m_key_images);

        return db_tx->put<crypto_key_image_t, std::vector<uint8_t>>(key_image, {});
    }

    Error BlockchainStorage::put_transaction(
        std::unique_ptr<Database::LMDBTransaction> &db_tx,
        const Types::Blockchain::transaction_t &transaction)
    {
        db_tx->set_database(m_transactions);

        /**
         * The reason that this looks so convoluted is because of the use
         * of std::variant for the different types of transactions
         */
        {
            auto error = std::visit(
                [this, &db_tx](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    {
                        // push the transaction itself into the database
                        auto error = db_tx->put(arg.hash(), arg.serialize());

                        if (error)
                        {
                            return error;
                        }
                    }

                    // handle the key images on each type of transaction
                    if constexpr (
                        std::is_same_v<
                            T,
                            Types::Blockchain::
                                committed_normal_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_stake_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_recall_stake_transaction_t>)
                    {
                        // loop through the key images in the transaction and push them into the database
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

        /**
         * The reason that this looks so convoluted is because of the use
         * of std::variant for the different types of transactions
         */
        {
            auto error = std::visit(
                [this, &count, &db_tx](auto &&arg) {
                    using T = std::decay_t<decltype(arg)>;

                    // handle the different types of transactions
                    if constexpr (
                        std::is_same_v<
                            T,
                            Types::Blockchain::
                                committed_normal_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_stake_transaction_t> || std::is_same_v<T, Types::Blockchain::committed_recall_stake_transaction_t> || std::is_same_v<T, Types::Blockchain::genesis_transaction_t>)
                    {
                        const auto txn_hash = arg.hash();

                        // used to keep track of the output indexes for storage later
                        std::vector<uint64_t> transaction_output_indexes;

                        // loop through the outputs and push them into the database for the global indexes
                        for (const auto &output : arg.outputs)
                        {
                            auto [error, index] = put_transaction_output(db_tx, count, output);

                            if (error)
                            {
                                return error;
                            }

                            std::cout << index << std::endl;

                            count++;

                            transaction_output_indexes.push_back(index);
                        }

                        // push the transaction global indexes into the database
                        auto error = put_transaction_indexes(db_tx, txn_hash, transaction_output_indexes);

                        if (error)
                        {
                            return error;
                        }
                    }
                    /**
                     * This transaction type is a snowflake in that it does not have an output subset as
                     * this type of transaction only ever contains a single output
                     */
                    else if constexpr (std::is_same_v<T, Types::Blockchain::stake_refund_transaction_t>)
                    {
                        const auto txn_hash = arg.hash();

                        std::vector<uint64_t> transaction_output_indexes;

                        /**
                         * We set the amount to 0 here as a) the amount is masked anyways and b) it does not
                         * matter for generating or checking signatures
                         */
                        const auto output =
                            Types::Blockchain::transaction_output_t(arg.public_ephemeral, 0, arg.commitment);

                        // push the output into the database
                        auto [error, index] = put_transaction_output(db_tx, count, output);

                        if (error)
                        {
                            return error;
                        }

                        count++;

                        transaction_output_indexes.push_back(index);

                        // push the transaction global indexes into the database
                        auto index_error = put_transaction_indexes(db_tx, txn_hash, transaction_output_indexes);

                        if (index_error)
                        {
                            return index_error;
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

        return SUCCESS;
    }

    Error BlockchainStorage::put_transaction_indexes(
        std::unique_ptr<Database::LMDBTransaction> &db_tx,
        const crypto_hash_t &tx_hash,
        const std::vector<uint64_t> &indexes)
    {
        db_tx->set_database(m_transaction_indexes);

        serializer_t writer;

        // write the indexes to a bytestream for easier storage
        for (const auto &index : indexes)
        {
            writer.varint(index);
        }

        return db_tx->put(tx_hash, writer.vector());
    }

    std::tuple<Error, uint64_t> BlockchainStorage::put_transaction_output(
        std::unique_ptr<Database::LMDBTransaction> &db_tx,
        const uint64_t &index,
        const Types::Blockchain::transaction_output_t &output)
    {
        db_tx->set_database(m_global_indexes);

        /**
         * We set the amount to 0 here as a) the amount is masked anyways and b) it does not
         * matter for generating or checking signatures
         */
        const auto temp = Types::Blockchain::transaction_output_t(output.public_ephemeral, 0, output.commitment);

        auto error = db_tx->put(index, temp.serialize_output());

        if (error)
        {
            return {error, 0};
        }

        return {SUCCESS, index};
    }
} // namespace Core
