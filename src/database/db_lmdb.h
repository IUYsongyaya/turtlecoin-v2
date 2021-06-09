// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef DATABASE_LMDB_H
#define DATABASE_LMDB_H

#include <crypto.h>
#include <lmdb.h>
#include <map>
#include <memory>
#include <mutex>
#include <tuple>

#define MDB_STR_ERR(variable) std::string(mdb_strerror(variable))
#define MDB_VAL(input, output) MDB_val output = {(input).size(), (void *)(input).data()}
#define MDB_VAL_NUM(input, output) MDB_val output = {sizeof(input), (void *)&(input)}
#define FROM_MDB_VAL(value)                                  \
    std::vector<uint8_t>(                                    \
        static_cast<const unsigned char *>((value).mv_data), \
        static_cast<const unsigned char *>((value).mv_data) + (value).mv_size)

namespace TurtleCoin::Database
{
    // forward declarations
    class LMDBDatabase;
    class LMDBTransaction;
    class LMDBCursor;

    /**
     * Wraps the LMDB C API into an OOP model that allows for opening and using
     * multiple environments and databases at once.
     */
    class LMDB
    {
      public:
        /**
         * Constructs an new instance of the LMDB wrapper.
         *
         * DO NOT USE THIS METHOD: Please use the static method getInstance to ensure
         * that an environment is only opened once in the entire process otherwise
         * you will run into issues.
         *
         * @param path
         * @param flags
         * @param mode
         * @param growth_factor in MB
         * @param max_databases
         */
        LMDB(
            const std::string &path,
            int flags = 0,
            int mode = 0600,
            size_t growth_factor = 8,
            unsigned int max_databases = 8);

        ~LMDB();

        operator MDB_env * &();

        /**
         * Closes the environment
         */
        void close();

        /**
         * Detects the current memory map size if it has been changed elsewhere
         * This requires that there are no open R/W transactions; otherwise, the method
         * will throw an exception.
         */
        void detect_map_size();

        /**
         * Expands the memory map by the growth factor supplied to the constructor
         * This requires that there are no open R/W transactions; otherwise, the method
         * will throw an exception.
         */
        void expand();

        /**
         * Expands the memory map by the number of pages specified.
         * This requires that there are no open R/W transactions; otherwise, the method
         * will throw an exception.
         * @param pages
         */
        void expand(size_t pages);

        /**
         * Flush the data buffers to disk.
         * Data is always written to disk when a transaction is committed, but the operating system may keep it
         * buffered. LMDB always flushes the OS buffers upon commit as well, unless the environment was opened
         * with MDB_NOSYNC or in part MDB_NOMETASYNC.
         * This call is not valid if the environment was opened with MDB_RDONLY.
         * @param force a synchronous flush of the buffers to disk
         */
        void flush(bool force = false);

        /**
         * Retrieves an already open database by its ID
         * @param id
         * @return
         */
        std::shared_ptr<LMDBDatabase> get_database(const std::string &id);

        /**
         * Retrieves the LMDB environment flags
         * @return
         */
        unsigned int get_flags();

        /**
         * Retrieves an existing instance of an environment by its ID
         * @param id
         * @return
         */
        static std::shared_ptr<LMDB> get_instance(const std::string &id);

        /**
         * Opens a LMDB environment using the specified parameters
         * @param path
         * @param flags
         * @param mode
         * @param growth_factor in MB
         * @param max_databases
         * @return
         */
        static std::shared_ptr<LMDB> getInstance(
            const std::string &path,
            int flags = 0,
            int mode = 0600,
            size_t growth_factor = 8,
            unsigned int max_databases = 8);

        /**
         * Retrieves the current environment growth factor (in MB)
         * @return
         */
        size_t growth_factor() const;

        /**
         * Retrieves the environments ID
         * @return
         */
        std::string id();

        /**
         * Retrieves the LMDB environment information
         * @return
         */
        MDB_envinfo info();

        /**
         * Retrieves the maximum byte size of a key in the LMDB environment
         * @return
         */
        size_t max_key_size();

        /**
         * Retrieves the maximum number of readers for the LMDB environment
         * @return
         */
        unsigned int max_readers();

        /**
         * Opens a database (separate key space) in the environment as a logical
         * partitioning of data.
         * @param name
         * @param flags
         * @return
         */
        std::shared_ptr<LMDBDatabase> open_database(const std::string &name, int flags = MDB_CREATE);

        /**
         * Returns the number of open R/W transactions in the environment
         * @return
         */
        size_t open_transactions();

        /**
         * Sets/changes the LMDB environment flags
         * @param flags
         * @param flag_state
         */
        void set_flags(int flags = 0, bool flag_state = true);

        /**
         * Retrieves the LMDB environment statistics
         * @return
         */
        MDB_stat stats();

        /**
         * Opens a transaction in the database
         * @param readonly
         * @return
         */
        std::unique_ptr<LMDBTransaction> transaction(bool readonly = false);

        /**
         * Registers a new transaction in the environment
         *
         * DO NOT USE THIS METHOD DIRECTLY!
         *
         * @param txn
         */
        void transaction_register(const LMDBTransaction &txn);

        /**
         * Un-registers a transaction from the environment
         *
         * DO NOT USE THIS METHOD DIRECTLY!
         *
         * @param txn
         */
        void transaction_unregister(const LMDBTransaction &txn);

        /**
         * Retrieves the current LMDB library version
         * @return [major, minor, patch]
         */
        static std::tuple<int, int, int> version();

      private:
        /**
         * Converts the bytes of memory specified into LMDB pages (rounded up)
         * @param memory
         * @return
         */
        size_t memory_to_pages(size_t memory);

        std::string m_id;

        size_t m_growth_factor;

        MDB_env *m_env;

        std::mutex m_mutex, m_txn_mutex;

        std::map<std::string, std::shared_ptr<LMDBDatabase>> m_databases;

        size_t m_open_txns;
    };

    /**
     * Provides a Database model for use within an LMDB environment
     */
    class LMDBDatabase
    {
      public:
        /**
         * Opens the database within the specified environment
         *
         * DO NOT CALL THIS METHOD DIRECTLY!
         *
         * @param env
         * @param name
         * @param flags
         */
        LMDBDatabase(std::shared_ptr<LMDB> &env, const std::string &name = "", int flags = MDB_CREATE);

        ~LMDBDatabase();

        operator MDB_dbi &();

        /**
         * Returns how many key/value pairs currently exist in the database
         * @return
         */
        size_t count();

        /**
         * Simplified deletion of the given key and its value. Automatically opens a
         * transaction, deletes the key, and commits the transaction, then returns.
         *
         * If we encounter MDB_MAP_FULL, we will automatically retry the transaction after
         * attempting to expand the database
         *
         * @tparam KeyType
         * @param key
         * @return
         */
        template<typename KeyType> int del(const KeyType &key);

        /**
         * Simplified deletion of the given key with the given value. Automatically
         * opens a transaction, deletes the value, and commits the transaction, then
         * returns.
         *
         * If we encounter MDB_MAP_FULL, we will automatically retry the transaction after
         * attempting to expand the database
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @param value
         * @return
         */
        template<typename KeyType, typename ValueType> int del(const KeyType &key, const ValueType &value);

        /**
         * Empties all of the key/value pairs from the database
         * @param delete_db if specified, also deletes the database from the environment
         * @return
         */
        bool drop(bool delete_db);

        /**
         * Returns the current LMDB environment associated with this database
         * @return
         */
        std::shared_ptr<LMDB> env();

        /**
         * Returns if the given key exists in the database
         * @tparam KeyType
         * @param key
         * @return
         */
        template<typename KeyType> bool exists(const KeyType &key);

        /**
         * Simplified retrieval of the value at the specified key which opens a new
         * readonly transaction, retrieves the value, and then returns it as the
         * specified type
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @return
         */
        template<typename KeyType, typename ValueType> std::tuple<bool, ValueType> get(const KeyType &key);

        /**
         * Simplified retrieval of the value at the specified key which opens a new
         * readonly transaction, retrieves the value, and then returns it as the
         * specified type
         *
         * @tparam ValueType
         * @param key
         * @return
         */
        template<typename ValueType> std::tuple<bool, ValueType> get(const uint64_t &key);

        /**
         * Simplified retrieval of the value at the specified key which opens a new
         * readonly transaction, retrieves the value, and then returns it
         *
         * @tparam KeyType
         * @param key
         * @return
         */
        template<typename KeyType> std::tuple<bool, std::vector<uint8_t>> get(const KeyType &key);

        /**
         * Retrieves the database flags
         * @return
         */
        unsigned int get_flags();

        /**
         * List all keys in the database
         * @tparam KeyType
         * @param ignore_duplicates whether we should ignore duplicate keys
         * @return
         */
        template<typename KeyType> std::vector<KeyType> list_keys(bool ignore_duplicates = true);

        /**
         * Simplified batch put which opens a new transaction, puts the value, and then returns.
         *
         * If we encounter MDB_MAP_FULL, we will automatically retry the transaction after
         * attempting to expand the database
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param keys
         * @param values
         * @return
         */
        template<typename KeyType, typename ValueType>
        int put(const std::vector<KeyType> &keys, const std::vector<ValueType> &values);

        /**
         * Simplified put which opens a new transaction, puts the value, and then returns.
         *
         * If we encounter MDB_MAP_FULL, we will automatically retry the transaction after
         * attempting to expand the database
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @param value
         * @return
         */
        template<typename KeyType, typename ValueType> int put(const KeyType &key, const ValueType &value);

        /**
         * Returns the ID of the database
         * @return
         */
        std::string id();

        /**
         * Opens a transaction in the database
         * @param readonly
         * @return
         */
        std::unique_ptr<LMDBTransaction> transaction(bool readonly = false);

      private:
        std::string m_id;

        std::shared_ptr<LMDB> m_env;

        MDB_dbi m_dbi;

        std::mutex m_db_mutex;
    };

    /**
     * Provides a transaction model for use within a LMDB database
     *
     * Please note: A transaction will abort() automatically if it has not been committed before
     * it leaves the scope it was created in. This helps to maintain database integrity as if
     * your work in pushing to a transaction fails and throws, the transaction will always be
     * reverted.
     *
     */
    class LMDBTransaction
    {
      public:
        /**
         * Constructs a new transaction in the environment specified
         *
         * DO NOT CALL THIS METHOD DIRECTLY!
         *
         * @param env
         * @param readonly
         */
        LMDBTransaction(std::shared_ptr<LMDB> &env, bool readonly = false);

        /**
         * Constructs a new transaction in the environment and database specified
         *
         * DO NOT CALL THIS METHOD DIRECTLY!
         *
         * @param env
         * @param db
         * @param readonly
         */
        LMDBTransaction(std::shared_ptr<LMDB> &env, std::shared_ptr<LMDBDatabase> &db, bool readonly = false);

        ~LMDBTransaction();

        operator MDB_txn * &();

        /**
         * Aborts the currently open transaction
         */
        void abort();

        /**
         * Commits the currently open transaction
         * @return
         */
        int commit();

        /**
         * Opens a LMDB cursos within the transaction
         * @return
         */
        std::unique_ptr<LMDBCursor> cursor();

        /**
         * Deletes the provided key
         * @tparam KeyType
         * @param key
         * @return
         */
        template<typename KeyType> int del(const KeyType &key)
        {
            MDB_VAL(key, i_key);

            return mdb_del(*m_txn, *m_db, &i_key, nullptr) == 0;
        }

        /**
         * Deletes the provided key
         * @param key
         * @return
         */
        int del(const uint64_t &key)
        {
            MDB_VAL_NUM(key, i_key);

            return mdb_del(*m_txn, *m_db, &i_key, nullptr) == 0;
        }

        /**
         * Deletes the provided key with the provided value.
         *
         * If the database supports sorted duplicates and the data parameter is NULL, all of the duplicate data items
         * for the key will be deleted. Otherwise, if the data parameter is non-NULL only the matching data item will
         * be deleted.
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @param value
         * @return
         */
        template<typename KeyType, typename ValueType> int del(const KeyType &key, const ValueType &value)
        {
            MDB_VAL(key, i_key);

            MDB_VAL(value, i_value);

            return mdb_del(*m_txn, *m_db, &i_key, &i_value);
        }

        /**
         * Deletes the provided key with the provided value.
         *
         * If the database supports sorted duplicates and the data parameter is NULL, all of the duplicate data items
         * for the key will be deleted. Otherwise, if the data parameter is non-NULL only the matching data item will
         * be deleted.
         *
         * @tparam ValueType
         * @param key
         * @param value
         * @return
         */
        template<typename ValueType> int del(const uint64_t &key, const ValueType &value)
        {
            MDB_VAL_NUM(key, i_key);

            MDB_VAL(value, i_value);

            return mdb_del(*m_txn, *m_db, &i_key, &i_value);
        }

        /**
         * Checks if the given key exists in the database
         * @tparam KeyType
         * @param key
         * @return
         */
        template<typename KeyType> bool exists(const KeyType &key)
        {
            const auto [found, value] = get(key);

            return found;
        }

        /**
         * Retrieves the value stored with the specified key as the specified type
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @return
         */
        template<typename KeyType, typename ValueType> std::tuple<bool, ValueType> get(const KeyType &key)
        {
            const auto [success, data] = get(key);

            ValueType result;

            if (success)
            {
                result = ValueType(data);
            }

            return {success, result};
        }

        /**
         * Retrieves the value stored with the specified key as the specified type
         * @tparam ValueType
         * @param key
         * @return
         */
        template<typename ValueType> std::tuple<bool, ValueType> get(const uint64_t &key)
        {
            MDB_VAL_NUM(key, i_key);

            MDB_val value;

            const auto success = mdb_get(*m_txn, *m_db, &i_key, &value);

            std::vector<uint8_t> data;

            ValueType _value;

            if (success == 0)
            {
                data = FROM_MDB_VAL(value);

                _value = ValueType(data);
            }

            return {success, _value};
        }

        /**
         * Retrieves the value stored with the specified key
         * @tparam KeyType
         * @param key
         * @return [found, value]
         */
        template<typename KeyType> std::tuple<bool, std::vector<uint8_t>> get(const KeyType &key)
        {
            MDB_VAL(key, i_key);

            MDB_val value;

            const auto success = mdb_get(*m_txn, *m_db, &i_key, &value);

            std::vector<uint8_t> result;

            if (success == 0)
            {
                result = FROM_MDB_VAL(value);
            }

            return {success == 0, result};
        }

        /**
         * Returns the transaction ID
         *
         * If a 0 value is returned, the transaction is complete [abort() or commit() has been used]
         * @return
         */
        size_t id();

        /**
         * Puts the specified value with the specified key in the database using the specified flag(s)
         *
         * Note: You must check for MDB_MAP_FULL or MDB_TXN_FULL response values and handle those
         * yourself as you will very likely need to abort the current transaction and expand
         * the LMDB environment before re-attempting the transaction.
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @param value
         * @param flags
         * @return
         */
        template<typename KeyType, typename ValueType>
        int put(const KeyType &key, const ValueType &value, int flags = 0)
        {
            MDB_VAL(key, i_key);

            MDB_VAL(value, i_value);

            return mdb_put(*m_txn, *m_db, &i_key, &i_value, flags);
        }

        /**
         * Puts the specified value with the specified key in the database using the specified flag(s)
         *
         * Note: You must check for MDB_MAP_FULL or MDB_TXN_FULL response values and handle those
         * yourself as you will very likely need to abort the current transaction and expand
         * the LMDB environment before re-attempting the transaction.
         *
         * @tparam ValueType
         * @param key
         * @param value
         * @param flags
         * @return
         */
        template<typename ValueType> int put(const uint64_t &key, const ValueType &value, int flags = 0)
        {
            MDB_VAL_NUM(key, i_key);

            MDB_VAL(value, i_value);

            return mdb_put(*m_txn, *m_db, &i_key, &i_value, flags);
        }

        /**
         * Returns if the transaction is readonly or not
         * @return
         */
        bool readonly() const;

        /**
         * Renew a read-only transaction that has been previously reset()
         *
         * @return
         */
        bool renew();

        /**
         * Reset a read-only transaction.
         */
        void reset();

        /**
         * Sets the current database for the transaction
         * @param db
         */
        void set_database(std::shared_ptr<LMDBDatabase> &db);

      private:
        /**
         * Sets up the transaction via the multiple entry methods
         */
        void txn_setup();

        std::shared_ptr<LMDB> m_env;

        std::shared_ptr<LMDBDatabase> m_db;

        std::shared_ptr<MDB_txn *> m_txn;

        bool m_readonly;
    };

    /**
     * Provies a Cursor model for use within a LMDB transaction
     */
    class LMDBCursor
    {
      public:
        /**
         * Creates a new LMDB Cursor within the specified transaction and database
         *
         * DO NOT CALL THIS METHOD DIRECTLY!
         *
         * @param txn
         * @param db
         * @param readonly
         */
        LMDBCursor(std::shared_ptr<MDB_txn *> &txn, std::shared_ptr<LMDBDatabase> &db, bool readonly = false);

        ~LMDBCursor();

        operator MDB_cursor * &();

        /**
         * Return count of duplicates for current key.
         * @return
         */
        size_t count();

        /**
         * Delete current key/data pair.
         * @param flags
         * @return
         */
        bool del(int flags = 0);

        /**
         * Retrieve key/data pairs by cursor.
         * @param op
         * @return [found, key, value]
         */
        std::tuple<bool, std::vector<uint8_t>, std::vector<uint8_t>> get(const MDB_cursor_op &op = MDB_FIRST);

        /**
         * Retrieve key/data pairs by cursor.
         * @tparam KeyType
         * @tparam ValueType
         * @param op
         * @return [found, key, value]
         */
        template<typename KeyType, typename ValueType>
        std::tuple<bool, KeyType, ValueType> get(const MDB_cursor_op &op = MDB_FIRST)
        {
            const auto [success, r_key, r_value] = get(op);

            KeyType key;

            ValueType value;

            if (success)
            {
                key = KeyType(r_key);

                value = ValueType(r_value);
            }

            return {success, key, value};
        }

        /**
         * Retrieve key/data pairs by cursor.
         *
         * Providing the key, allows for utilizing additional MDB_cursor_op values such as MDB_SET which will
         * retrieve the value for the specified key without changing the key value.
         *
         * @tparam KeyType
         * @param key
         * @param op
         * @return [found, key, value]
         */
        template<typename KeyType>
        std::tuple<bool, std::vector<uint8_t>, std::vector<uint8_t>>
            get(const KeyType &key, const MDB_cursor_op &op = MDB_SET)
        {
            MDB_val i_value;

            MDB_VAL(key, i_key);

            const auto success = mdb_cursor_get(m_cursor, &i_key, &i_value, op);

            std::vector<uint8_t> r_key, r_value;

            if (success == 0)
            {
                r_key = FROM_MDB_VAL(i_key);

                r_value = FROM_MDB_VAL(i_value);
            }

            return {success == 0, r_key, r_value};
        }

        /**
         * Retrieve key/data pairs by cursor.
         *
         * Providing the key, allows for utilizing additional MDB_cursor_op values such as MDB_SET which will
         * retrieve the value for the specified key without changing the key value.
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @param op
         * @return [found, key, value]
         */
        template<typename KeyType, typename ValueType>
        std::tuple<bool, KeyType, ValueType> get(const KeyType &key, const MDB_cursor_op &op = MDB_SET)
        {
            const auto [success, i_key, i_value] = get(key, op);

            KeyType r_key;

            ValueType r_value;

            if (success)
            {
                r_key = KeyType(i_key);

                r_value = ValueType(i_value);
            }

            return {success, r_key, r_value};
        }

        /**
         * Retrieve multiple values for a single key from the database
         *
         * Requires that MDB_DUPSORT was used when opening the database
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @return [found, key, values]
         */
        template<typename KeyType, typename ValueType>
        std::tuple<bool, KeyType, std::vector<ValueType>> get_all(const KeyType &key)
        {
            std::vector<ValueType> results;

            bool success = false;

            do
            {
                const auto [found, k, v] = get<KeyType, ValueType>(key, (!success) ? MDB_SET : MDB_NEXT_DUP);

                if (found)
                {
                    results.push_back(v);
                }

                success = found;
            } while (success);

            return {!results.empty(), key, results};
        }

        /**
         * Puts the specified value with the specified key in the database using the specified flag(s)
         * and places the cursor at the position of the new item or, near it upon failure.
         *
         * Note: You must check for MDB_MAP_FULL or MDB_TXN_FULL response values and handle those
         * yourself as you will very likely need to abort the current transaction and expand
         * the LMDB environment before re-attempting the transaction.
         *
         * @tparam KeyType
         * @tparam ValueType
         * @param key
         * @param value
         * @param flags
         * @return
         */
        template<typename KeyType, typename ValueType>
        int put(const KeyType &key, const ValueType &value, int flags = 0)
        {
            MDB_VAL(key, i_key);

            MDB_VAL(value, i_value);

            return mdb_cursor_put(m_cursor, &i_key, &i_value, flags);
        }

        /**
         * Renews the cursor
         * @return
         */
        bool renew();

      private:
        std::shared_ptr<LMDBDatabase> m_db;

        std::shared_ptr<MDB_txn *> m_txn;

        MDB_cursor *m_cursor;

        bool m_readonly;
    };

    /**
     * Complete the template forward declarations
     */

    template<typename KeyType> int LMDBDatabase::del(const KeyType &key)
    {
    try_again:
        auto txn = transaction();

        auto success = txn->del(key);

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        success = txn->commit();

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        return success;
    }

    template<typename KeyType, typename ValueType> int LMDBDatabase::del(const KeyType &key, const ValueType &value)
    {
    try_again:
        auto txn = transaction();

        auto success = txn->del(key, value);

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        success = txn->commit();

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        return success;
    }

    template<typename KeyType> bool LMDBDatabase::exists(const KeyType &key)
    {
        auto txn = transaction(true);

        return txn->exists(key);
    }

    template<typename KeyType, typename ValueType> std::tuple<bool, ValueType> LMDBDatabase::get(const KeyType &key)
    {
        const auto [success, data] = get(key);

        ValueType result;

        if (success)
        {
            result = ValueType(data);
        }

        return {success, result};
    }

    template<typename ValueType> std::tuple<bool, ValueType> LMDBDatabase::get(const uint64_t &key)
    {
        auto txn = transaction(true);

        return txn->get<ValueType>(key);
    }

    template<typename KeyType> std::tuple<bool, std::vector<uint8_t>> LMDBDatabase::get(const KeyType &key)
    {
        auto txn = transaction(true);

        return txn->get(key);
    }

    template<typename KeyType> std::vector<KeyType> LMDBDatabase::list_keys(bool ignore_duplicates)
    {
        auto txn = transaction(true);

        auto cursor = txn->cursor();

        auto db_cursor = *cursor;

        std::vector<KeyType> results;

        MDB_val key, value;

        size_t count = 0;

        auto last_key = std::string();

        while (mdb_cursor_get(db_cursor, &key, &value, count ? MDB_NEXT : MDB_FIRST) == 0)
        {
            const auto bytes = FROM_MDB_VAL(key);

            const auto val = Crypto::StringTools::to_hex(bytes.data(), bytes.size());

            if (ignore_duplicates && val == last_key)
            {
                continue;
            }

            results.push_back(KeyType(bytes));

            last_key = val;

            count++;
        }

        return results;
    }

    template<typename KeyType, typename ValueType>
    int LMDBDatabase::put(const std::vector<KeyType> &keys, const std::vector<ValueType> &values)
    {
        if (keys.size() != values.size())
            throw std::invalid_argument("keys and values must be of the same size");

    try_again:
        auto txn = transaction();

        for (size_t i = 0; i < keys.size(); ++i)
        {
            const auto success = txn->put(keys[i], values[i]);

            if (success == MDB_MAP_FULL)
            {
                txn->abort();

                m_env->expand();

                goto try_again;
            }
        }

        const auto success = txn->commit();

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        return success;
    }

    template<typename KeyType, typename ValueType> int LMDBDatabase::put(const KeyType &key, const ValueType &value)
    {
    try_again:
        auto txn = transaction();

        auto success = txn->put(key, value);

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        success = txn->commit();

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        return success;
    }
} // namespace TurtleCoin::Database

#endif // DATABASE_LMDB_H
