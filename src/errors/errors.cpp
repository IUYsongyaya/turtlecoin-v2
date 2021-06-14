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
            writer.Uint(m_error_code);

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
        case GENERIC_FAILURE:
            return "A generic failure occurred";
        case BASE58_DECODE:
            return "Could not decode Base58 string.";
        case ADDRESS_PREFIX_MISMATCH:
            return "The address prefix did not match the expected result.";
        case NOT_A_PUBLIC_KEY:
            return "The public key supplied is not a point on the curve.";
        case LMDB_ERROR:
            return "The LMDB operation failed. Please report this error as this default text should be replaced by "
                   "more detailed information.";
        case DB_KEY_NOT_FOUND:
            return "The database key requested could not be found.";
        case UNKNOWN_TRANSACTION_TYPE:
            return "The transaction type encountered is of an unknown type and cannot be handled.";
        case GLOBAL_INDEX_OUT_OF_BOUNDS:
            return "The global index specified is out of the range of the global indexes.";
        case BLOCK_NOT_FOUND:
            return "The block requested from the database cannot be found.";
        case TRANSACTION_NOT_FOUND:
            return "The transaction requested from the database cannot be found.";
        case BLOCK_DOES_NOT_CHAIN:
            return "The block supplied does not chain to a previous block.";
        case BLOCK_ALREADY_EXISTS:
            return "The block for the height provided already exists in storage.";
        case BLOCK_TXN_ORDER:
            return "The transactions supplied for the DB are not in the same order as specified in the block.";
        case STAKING_CANDIDATE_NOT_FOUND:
            return "The staking candidate was not found in the database.";
        case STAKING_STAKER_NOT_FOUND:
            return "The staker was not found in the database.";
        case DESERIALIZATION_ERROR:
            return "Deserialization error encountered.";
        case BLOCK_TRANSACTIONS_MISMATCH:
            return "The transactions specified do not match those within the block.";
        default:
            return "Unknown error code supplied";
    }
}
