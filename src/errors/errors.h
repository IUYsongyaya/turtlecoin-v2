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
    BLOCK_TXN_ORDER
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

    ErrorCode code() const;

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
