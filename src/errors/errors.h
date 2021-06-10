// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_ERRORS_H
#define TURTLECOIN_ERRORS_H

#include <sstream>
#include <string>

enum ErrorCode
{
    SUCCESS = 0,
    BASE58_DECODE,
    ADDRESS_PREFIX_MISMATCH,
    NOT_A_PUBLIC_KEY,
    LMDB_ERROR,
    DB_KEY_NOT_FOUND,
    UNKNOWN_TRANSACTION_TYPE,
    GLOBAL_INDEX_OUT_OF_BOUNDS,
    BLOCK_NOT_FOUND,
    TRANSACTION_NOT_FOUND,
    BLOCK_DOES_NOT_CHAIN,
    BLOCK_ALREADY_EXISTS,
    BLOCK_TXN_ORDER,
    STAKING_CANDIDATE_NOT_FOUND,
    STAKING_STAKER_NOT_FOUND,
    DESERIALIZATION_ERROR,
    BLOCK_TRANSACTIONS_MISMATCH,
    /**
     * Do not change LMDB values as they map directly to LMDB return codes
     * See: http://www.lmdb.tech/doc/group__errors.html
     */
    LMDB_KEYEXIST = -30799,
    LMDB_NOTFOUND = -30798,
    LMDB_PAGE_NOTFOUND = -30797,
    LMDB_CORRUPTED = -30796,
    LMDB_PANIC = -30795,
    LMDB_VERSION_MISMATCH = -30794,
    LMDB_INVALID = -30793,
    LMDB_MAP_FULL = -30792,
    LMDB_DBS_FULL = -30791,
    LMDB_READERS_FULL = -30790,
    LMDB_TLS_FULL = -30789,
    LMDB_TXN_FULL = -30788,
    LMDB_CURSOR_FULL = -30787,
    LMDB_PAGE_FULL = -30786,
    LMDB_MAP_RESIZED = -30785,
    LMDB_INCOMPATIBLE = -30784,
    LMDB_BAD_RSLOT = -30783,
    LMDB_BAD_TXN = -30782,
    LMDB_BAD_VALSIZE = -30781,
    LMDB_BAD_DBI = -30780
};

class Error
{
  public:
    Error(): m_error_code(SUCCESS) {}

    /**
     * Creates an error with the specified code
     *
     * @param code
     */
    Error(const ErrorCode &code): m_error_code(code) {}

    /**
     * Creates an error with the specified code and a custom error message
     *
     * @param code
     * @param custom_message
     */
    Error(const ErrorCode &code, const std::string &custom_message):
        m_error_code(code), m_custom_error_message(custom_message)
    {
    }

    /**
     * Creates an error with the specified code
     *
     * @param code
     */
    Error(const int &code): m_error_code(static_cast<ErrorCode>(code)) {}

    /**
     * Creates an error with the specified code and a custom error message
     *
     * @param code
     * @param custom_message
     */
    Error(const int &code, const std::string &custom_message):
        m_error_code(static_cast<ErrorCode>(code)), m_custom_error_message(custom_message)
    {
    }

    bool operator==(const ErrorCode &code) const
    {
        return code == m_error_code;
    }

    bool operator==(const Error &error) const
    {
        return error.code() == m_error_code;
    }

    bool operator!=(const ErrorCode &code) const
    {
        return !(code == m_error_code);
    }

    bool operator!=(const Error &error) const
    {
        return !(error.code() == m_error_code);
    }

    explicit operator bool() const
    {
        return m_error_code != SUCCESS;
    }

    /**
     * Returns the error code
     *
     * @return
     */
    ErrorCode code() const;

    /**
     * Returns the error message of the instance
     *
     * @return
     */
    std::string to_string() const;

  private:
    ErrorCode m_error_code;

    std::string m_custom_error_message;
};

inline std::ostream &operator<<(std::ostream &os, const Error &error)
{
    os << "Error #" << std::to_string(error.code()) << ": " << error.to_string();

    return os;
}

#endif // TURTLECOIN_ERRORS_H
