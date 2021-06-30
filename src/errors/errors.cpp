// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "errors.h"

ErrorCode Error::code() const
{
    return m_error_code;
}

std::string Error::file_name() const
{
    return m_file_name;
}

size_t Error::line() const
{
    return m_line_number;
}

JSON_TO_FUNC(Error::toJSON)
{
    writer.StartObject();
    {
        writer.Key("error");
        writer.StartObject();
        {
            writer.Key("code");
            writer.Int64(m_error_code);

            writer.Key("message");
            writer.String(to_string());
        }
        writer.EndObject();
    }
    writer.EndObject();
}

std::string Error::to_string() const
{
    if (!m_custom_error_message.empty())
    {
        return m_custom_error_message;
    }

    switch (m_error_code)
    {
        case SUCCESS:
            return "The operation completed successfully.";
        case DB_EMPTY:
            return "The database is empty";
        case BASE58_DECODE:
            return "Could not decode Base58 string.";
        case ADDRESS_PREFIX_MISMATCH:
            return "The address prefix did not match the expected result.";
        case LMDB_ERROR:
            return "The LMDB operation failed. Please report this error as this default text should be replaced by "
                   "more detailed information.";
        case LMDB_EMPTY:
            return "The LMDB database appears to be empty. The database may be legitimately empty or an underlying "
                   "issue persists in the database.";
        case UNKNOWN_TRANSACTION_TYPE:
            return "The transaction type encountered is of an unknown type and cannot be handled.";
        case BLOCK_TXN_ORDER:
            return "The transactions supplied for the DB are not in the same order as specified in the block.";
        case STAKING_CANDIDATE_NOT_FOUND:
            return "The staking candidate was not found in the database.";
        case STAKING_STAKER_NOT_FOUND:
            return "The staker was not found in the database.";
        default:
            return "The error code supplied does not have a default message. Please create one.";
    }
}
