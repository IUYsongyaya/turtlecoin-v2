// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_ERRORS_H
#define TURTLECOIN_ERRORS_H

#include <json_helper.h>
#include <sstream>
#include <string>

#define MAKE_ERROR(code) Error(code, __LINE__, __FILE__)
#define MAKE_ERROR_MSG(code, message) Error(code, message, __LINE__, __FILE__)

enum ErrorCode
{
    SUCCESS = 0,

    // (de)serialization error code(s)
    JSON_PARSE_ERROR,

    // networking error code(s)
    UPNP_FAILURE,
    UPNP_NOT_SUPPORTED,
    ZMQ_CONNECT_ERROR,
    ZMQ_BIND_ERROR,
    ZMQ_GENERIC_ERROR,
    P2P_SEED_CONNECT,
    P2P_DUPE_CONNECT,
    HTTP_BODY_REQUIRED_BUT_NOT_FOUND,

    // peer list error code(s)
    PEERLIST_ADD_FAILURE,

    // address encoding error code(s)
    BASE58_DECODE,
    ADDRESS_PREFIX_MISMATCH,
    ADDRESS_DECODE,

    // database error code(s)
    DB_EMPTY,
    DB_BLOCK_NOT_FOUND,
    DB_TRANSACTION_NOT_FOUND,
    DB_GLOBAL_INDEX_OUT_OF_BOUNDS,
    DB_DESERIALIZATION_ERROR,

    // block error code(s)
    BLOCK_TXN_ORDER,
    BLOCK_TXN_MISMATCH,

    // transaction error code(s)
    UNKNOWN_TRANSACTION_TYPE,

    // staking error code(s)
    STAKING_CANDIDATE_NOT_FOUND,
    STAKING_STAKER_NOT_FOUND,

    /**
     * Do not change LMDB values as they map directly to LMDB return codes
     * See: http://www.lmdb.tech/doc/group__errors.html
     */
    LMDB_ERROR = -40000,
    LMDB_EMPTY = -39999,
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
     * @param line_number
     * @param file_name
     */
    Error(const ErrorCode &code, size_t line_number = 0, const std::string file_name = ""):
        m_error_code(code), m_line_number(line_number), m_file_name(file_name)
    {
    }

    /**
     * Creates an error with the specified code and a custom error message
     *
     * @param code
     * @param custom_message
     * @param line_number
     * @param file_name
     */
    Error(
        const ErrorCode &code,
        const std::string &custom_message,
        size_t line_number = 0,
        const std::string file_name = ""):
        m_error_code(code), m_custom_error_message(custom_message), m_line_number(line_number), m_file_name(file_name)
    {
    }

    /**
     * Creates an error with the specified code
     *
     * @param code
     * @param line_number
     * @param file_name
     */
    Error(const int &code, size_t line_number = 0, const std::string file_name = ""):
        m_error_code(static_cast<ErrorCode>(code)), m_line_number(line_number), m_file_name(file_name)
    {
    }

    /**
     * Creates an error with the specified code and a custom error message
     *
     * @param code
     * @param custom_message
     * @param line_number
     * @param file_name
     */
    Error(const int &code, const std::string &custom_message, size_t line_number = 0, const std::string file_name = ""):
        m_error_code(static_cast<ErrorCode>(code)),
        m_custom_error_message(custom_message),
        m_line_number(line_number),
        m_file_name(file_name)
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
     * Return the filename of the file where the error was created
     *
     * @return
     */
    std::string file_name() const;

    /**
     * Return the line number of the file where the error was created
     *
     * @return
     */
    size_t line() const;

    /**
     * Provides the error as a JSON object
     *
     * @param writer
     */
    JSON_TO_FUNC(toJSON);

    /**
     * Returns the error message of the instance
     *
     * @return
     */
    std::string to_string() const;

  private:
    ErrorCode m_error_code;

    size_t m_line_number;

    std::string m_file_name;

    std::string m_custom_error_message;
};

inline std::ostream &operator<<(std::ostream &os, const Error &error)
{
    if (!error.file_name().empty())
    {
        os << error.file_name() << " L#" << error.line() << " ";
    }

    os << "Error #" << std::to_string(error.code()) << ": " << error.to_string();

    return os;
}

#endif // TURTLECOIN_ERRORS_H
