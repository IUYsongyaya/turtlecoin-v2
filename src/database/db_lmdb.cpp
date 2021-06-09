// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "db_lmdb.h"

#include <cppfs/FileHandle.h>
#include <cppfs/fs.h>
#include <utility>

#define LMDB_SPACE_MULTIPLIER (1024 * 1024) // to MB

std::map<std::string, std::shared_ptr<TurtleCoin::Database::LMDB>> l_environments;

namespace TurtleCoin::Database
{
    LMDB::LMDB(const std::string &path, int flags, int mode, size_t growth_factor, unsigned int max_databases):
        m_growth_factor(growth_factor), m_env(nullptr), m_open_txns(0)
    {
        m_id = Crypto::Hashing::sha3(path.data(), path.size()).to_string();

        auto dir = cppfs::fs::open(path);

        if (!dir.isDirectory())
        {
            dir.createDirectory();
        }

        auto success = mdb_env_create(&m_env);

        if (success != 0)
        {
            throw std::runtime_error("Could not create LMDB environment: " + MDB_STR_ERR(success));
        }

        success = mdb_env_set_mapsize(m_env, growth_factor * LMDB_SPACE_MULTIPLIER);

        if (success != 0)
        {
            throw std::runtime_error("Could not allocate initial LMDB memory map: " + MDB_STR_ERR(success));
        }

        success = mdb_env_set_maxdbs(m_env, max_databases);

        if (success != 0)
        {
            throw std::runtime_error("Could not set maximum number of LMDB named databases: " + MDB_STR_ERR(success));
        }

        /**
         * A transaction and its cursors must only be used by a single thread, and a thread may only have a single
         * transaction at a time. If MDB_NOTLS is in use, this does not apply to read-only transactions.
         */
        success = mdb_env_open(m_env, path.c_str(), flags | MDB_NOTLS, mode);

        if (success != 0)
        {
            mdb_env_close(m_env);

            throw std::runtime_error("Could not open LMDB database file: " + path + ": " + MDB_STR_ERR(success));
        }
    }

    LMDB::~LMDB()
    {
        close();
    }

    LMDB::operator MDB_env * &()
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        return m_env;
    }

    void LMDB::close()
    {
        std::scoped_lock lock(m_mutex);

        flush(true);

        mdb_env_close(m_env);

        m_env = nullptr;

        if (l_environments.find(m_id) != l_environments.end())
        {
            l_environments.erase(m_id);
        }
    }

    void LMDB::detect_map_size()
    {
        std::scoped_lock lock(m_mutex);

        if (open_transactions() != 0)
        {
            throw std::runtime_error("Cannot detect LMDB environment map size while transactions are open");
        }

        const auto success = mdb_env_set_mapsize(m_env, 0);

        if (success != 0)
        {
            throw std::runtime_error("Could not detect LMDB environment memory map size: " + MDB_STR_ERR(success));
        }
    }

    void LMDB::expand()
    {
        const auto pages = memory_to_pages(m_growth_factor * 1024 * 1024);

        expand(pages);
    }

    void LMDB::expand(size_t pages)
    {
        std::scoped_lock lock(m_mutex);

        if (open_transactions() != 0)
        {
            throw std::runtime_error("Cannot expand LMDB environment map size while transactions are open");
        }

        const auto l_info = info();

        const auto l_stats = stats();

        const auto new_size = (l_stats.ms_psize * pages) + l_info.me_mapsize;

        const auto success = mdb_env_set_mapsize(m_env, new_size);

        if (success != 0)
        {
            throw std::runtime_error("Could not expand LMDB environment memory map size: " + MDB_STR_ERR(success));
        }
    }

    void LMDB::flush(bool force)
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        const auto success = mdb_env_sync(m_env, (force) ? 1 : 0);

        if (success != 0)
        {
            throw std::runtime_error("Could not flush LMDB data buffers to disk: " + MDB_STR_ERR(success));
        }
    }

    std::shared_ptr<LMDBDatabase> LMDB::get_database(const std::string &id)
    {
        if (m_databases.find(id) != m_databases.end())
        {
            return m_databases.at(id);
        }

        throw std::invalid_argument("LMDB database not found");
    }

    unsigned int LMDB::get_flags()
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        unsigned int env_flags;

        const auto success = mdb_env_get_flags(m_env, &env_flags);

        if (success != 0)
        {
            throw std::runtime_error("Could not retrieve LMDB environment flags: " + MDB_STR_ERR(success));
        }

        return env_flags;
    }

    std::shared_ptr<LMDB> LMDB::get_instance(const std::string &id)
    {
        if (l_environments.find(id) != l_environments.end())
        {
            return l_environments.at(id);
        }

        throw std::invalid_argument("LMDB environment not found");
    }

    std::shared_ptr<LMDB> LMDB::getInstance(
        const std::string &path,
        int flags,
        int mode,
        size_t growth_factor,
        unsigned int max_databases)
    {
        auto id = Crypto::Hashing::sha3(path.data(), path.size()).to_string();

        if (l_environments.find(id) != l_environments.end())
        {
            return l_environments.at(id);
        }

        auto db = std::make_shared<LMDB>(path, flags, mode, growth_factor, max_databases);

        l_environments.insert({id, db});

        return db;
    }

    size_t LMDB::growth_factor() const
    {
        return m_growth_factor;
    }

    std::string LMDB::id()
    {
        return m_id;
    }

    MDB_envinfo LMDB::info()
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        MDB_envinfo info;

        const auto success = mdb_env_info(m_env, &info);

        if (success != 0)
        {
            throw std::runtime_error("Could not retrieve LMDB environment information: " + MDB_STR_ERR(success));
        }

        return info;
    }

    size_t LMDB::memory_to_pages(size_t memory)
    {
        const auto l_stats = stats();

        return size_t(ceil(double(memory) / double(l_stats.ms_psize)));
    }

    size_t LMDB::max_key_size()
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        return mdb_env_get_maxkeysize(m_env);
    }

    unsigned int LMDB::max_readers()
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        unsigned int readers;

        const auto success = mdb_env_get_maxreaders(m_env, &readers);

        if (success != 0)
        {
            throw std::runtime_error("Could not retrieve LMDB maximum readers: " + MDB_STR_ERR(success));
        }

        return readers;
    }

    std::shared_ptr<LMDBDatabase> LMDB::open_database(const std::string &name, int flags)
    {
        auto id = Crypto::Hashing::sha3(name.data(), name.size()).to_string();

        if (m_databases.find(id) != m_databases.end())
        {
            return m_databases.at(id);
        }

        auto env = LMDB::get_instance(m_id);

        auto db = std::make_shared<LMDBDatabase>(env, name, flags);

        m_databases.insert({id, db});

        return db;
    }

    size_t LMDB::open_transactions()
    {
        std::scoped_lock lock(m_txn_mutex);

        return m_open_txns;
    }

    void LMDB::set_flags(int flags, bool flag_state)
    {
        std::scoped_lock lock(m_mutex);

        const auto success = mdb_env_set_flags(m_env, flags, (flag_state) ? 1 : 0);

        if (success != 0)
        {
            throw std::runtime_error("Could not set LMDB environment flags: " + MDB_STR_ERR(success));
        }
    }

    MDB_stat LMDB::stats()
    {
        if (!m_env)
        {
            throw std::runtime_error("LMDB environment has been previously closed");
        }

        MDB_stat stats;

        const auto success = mdb_env_stat(m_env, &stats);

        if (success != 0)
        {
            throw std::runtime_error("Could not retrieve LMDB environment statistics: " + MDB_STR_ERR(success));
        }

        return stats;
    }

    std::unique_ptr<LMDBTransaction> LMDB::transaction(bool readonly)
    {
        auto instance = LMDB::get_instance(id());

        return std::make_unique<LMDBTransaction>(instance, readonly);
    }

    void LMDB::transaction_register(const LMDBTransaction &txn)
    {
        if (txn.readonly())
        {
            return;
        }

        std::scoped_lock lock(m_txn_mutex);

        m_open_txns++;
    }

    void LMDB::transaction_unregister(const LMDBTransaction &txn)
    {
        if (txn.readonly())
        {
            return;
        }

        std::scoped_lock lock(m_txn_mutex);

        if (m_open_txns > 0)
        {
            m_open_txns--;
        }
    }

    std::tuple<int, int, int> LMDB::version()
    {
        int major, minor, patch;

        mdb_version(&major, &minor, &patch);

        return {major, minor, patch};
    }

    LMDBDatabase::LMDBDatabase(std::shared_ptr<LMDB> &env, const std::string &name, int flags): m_env(env), m_dbi(0)
    {
        m_id = Crypto::Hashing::sha3(name.data(), name.size()).to_string();

        const auto env_flags = env->get_flags();

        const auto readonly = (env_flags & MDB_RDONLY);

        auto txn = std::make_unique<LMDBTransaction>(m_env, readonly);

        auto success = mdb_dbi_open(*txn, name.empty() ? nullptr : name.c_str(), flags, &m_dbi);

        if (success != 0)
        {
            throw std::runtime_error("Unable to open LMDB named database: " + MDB_STR_ERR(success));
        }

        if (!(env_flags & MDB_RDONLY))
        {
            success = txn->commit();

            if (success != 0)
            {
                throw std::runtime_error("Could not commit to open LMDB named database: " + MDB_STR_ERR(success));
            }
        }
    }

    LMDBDatabase::~LMDBDatabase()
    {
        mdb_dbi_close(*m_env, m_dbi);
    }

    LMDBDatabase::operator MDB_dbi &()
    {
        return m_dbi;
    }

    size_t LMDBDatabase::count()
    {
        auto db = m_env->get_database(m_id);

        auto txn = std::make_unique<LMDBTransaction>(m_env, db, true);

        auto cursor = txn->cursor();

        auto db_cursor = *cursor;

        MDB_val key, value;

        size_t count = 0;

        while (mdb_cursor_get(db_cursor, &key, &value, count ? MDB_NEXT : MDB_FIRST) == 0)
        {
            count++;
        }

        return count;
    }

    bool LMDBDatabase::drop(bool delete_db)
    {
        std::scoped_lock lock(m_db_mutex);

    try_again:

        auto txn = std::make_unique<LMDBTransaction>(m_env);

        const auto success = mdb_drop(*txn, m_dbi, (delete_db) ? 1 : 0);

        if (success == MDB_MAP_FULL)
        {
            txn->abort();

            m_env->expand();

            goto try_again;
        }

        return txn->commit() == 0;
    }

    std::shared_ptr<LMDB> LMDBDatabase::env()
    {
        return m_env;
    }

    unsigned int LMDBDatabase::get_flags()
    {
        auto txn = transaction(true);

        unsigned int dbi_flags;

        const auto success = mdb_dbi_flags(*txn, m_dbi, &dbi_flags);

        if (success != 0)
        {
            throw std::runtime_error("Could not retrieve LMDB database flags: " + MDB_STR_ERR(success));
        }

        return dbi_flags;
    }

    std::string LMDBDatabase::id()
    {
        return m_id;
    }

    std::unique_ptr<LMDBTransaction> LMDBDatabase::transaction(bool readonly)
    {
        if (m_dbi == 0)
        {
            throw std::runtime_error("LMDB database no longer exists");
        }

        std::scoped_lock lock(m_db_mutex);

        auto db = m_env->get_database(m_id);

        return std::make_unique<LMDBTransaction>(m_env, db, readonly);
    }

    LMDBTransaction::LMDBTransaction(std::shared_ptr<LMDB> &env, bool readonly): m_env(env), m_readonly(readonly)
    {
        txn_setup();
    }

    LMDBTransaction::LMDBTransaction(std::shared_ptr<LMDB> &env, std::shared_ptr<LMDBDatabase> &db, bool readonly):
        m_env(env), m_db(db), m_readonly(readonly)
    {
        txn_setup();
    }

    LMDBTransaction::~LMDBTransaction()
    {
        // default action is to abort if the Transaction leaves scope
        abort();
    }

    LMDBTransaction::operator MDB_txn * &()
    {
        return *m_txn;
    }

    void LMDBTransaction::abort()
    {
        if (!m_txn)
        {
            return;
        }

        mdb_txn_abort(*m_txn);

        m_env->transaction_unregister(*this);

        m_txn = nullptr;
    }

    int LMDBTransaction::commit()
    {
        if (!m_txn)
        {
            return MDB_BAD_TXN;
        }

        const auto success = mdb_txn_commit(*m_txn);

        m_env->transaction_unregister(*this);

        m_txn = nullptr;

        return success;
    }

    std::unique_ptr<LMDBCursor> LMDBTransaction::cursor()
    {
        return std::make_unique<LMDBCursor>(m_txn, m_db, m_readonly);
    }

    size_t LMDBTransaction::id()
    {
        if (!m_txn)
        {
            return 0;
        }

        return mdb_txn_id(*m_txn);
    }

    bool LMDBTransaction::readonly() const
    {
        return m_readonly;
    }

    bool LMDBTransaction::renew()
    {
        if (!m_txn || !m_readonly)
        {
            return false;
        }

        return mdb_txn_renew(*m_txn) == 0;
    }

    void LMDBTransaction::reset()
    {
        if (!m_txn || !m_readonly)
        {
            return;
        }

        mdb_txn_renew(*m_txn);
    }

    void LMDBTransaction::set_database(std::shared_ptr<LMDBDatabase> &db)
    {
        m_db = db;
    }

    void LMDBTransaction::txn_setup()
    {
        MDB_txn *result;

        for (int i = 0; i < 3; ++i)
        {
            const auto success = mdb_txn_begin(*m_env, nullptr, (m_readonly) ? MDB_RDONLY : 0, &result);

            if (success == 0)
            {
                break;
            }

            if (success == MDB_MAP_RESIZED && i < 2)
            {
                m_env->detect_map_size();

                continue;
            }

            throw std::runtime_error("Unable to start LMDB transaction: " + MDB_STR_ERR(success));
        }

        m_txn = std::make_shared<MDB_txn *>(result);

        m_env->transaction_register(*this);
    }

    LMDBCursor::LMDBCursor(std::shared_ptr<MDB_txn *> &txn, std::shared_ptr<LMDBDatabase> &db, bool readonly):
        m_txn(txn), m_db(db), m_readonly(readonly)
    {
        const auto success = mdb_cursor_open(*m_txn, *m_db, &m_cursor);

        if (success != 0)
        {
            throw std::runtime_error("Could not open LMDB cursor: " + MDB_STR_ERR(success));
        }
    }

    LMDBCursor::~LMDBCursor()
    {
        if (!m_cursor)
        {
            return;
        }

        mdb_cursor_close(m_cursor);
    }

    LMDBCursor::operator MDB_cursor * &()
    {
        return m_cursor;
    }

    size_t LMDBCursor::count()
    {
        if (!m_cursor)
        {
            return 0;
        }

        size_t count;

        const auto success = mdb_cursor_count(m_cursor, &count);

        if (success != 0)
        {
            throw std::runtime_error("Could not get the count from the LMDB cursor: " + MDB_STR_ERR(success));
        }

        return count;
    }

    bool LMDBCursor::del(int flags)
    {
        return mdb_cursor_del(m_cursor, flags) == 0;
    }

    std::tuple<bool, std::vector<uint8_t>, std::vector<uint8_t>> LMDBCursor::get(const MDB_cursor_op &op)
    {
        MDB_val i_key, i_value;

        const auto success = mdb_cursor_get(m_cursor, &i_key, &i_value, op);

        std::vector<uint8_t> r_key, r_value;

        if (success == 0)
        {
            r_key = FROM_MDB_VAL(i_key);

            r_value = FROM_MDB_VAL(i_value);
        }

        return {success == 0, r_key, r_value};
    }

    bool LMDBCursor::renew()
    {
        if (!m_cursor || !m_readonly)
        {
            return false;
        }

        return mdb_cursor_renew(*m_txn, m_cursor) == 0;
    }
} // namespace TurtleCoin::Database
