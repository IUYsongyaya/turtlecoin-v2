// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <types.h>

using namespace TurtleCoin::Types;

template<typename T> static inline void check_json_serialization(const T &value, const std::string &name = "")
{
    std::cout << "Checking [" << name << "] JSON serialization: ";

    JSON_INIT_BUFFER(buffer, writer);

    value.toJSON(writer);

    JSON_DUMP_BUFFER(buffer, encoded);

    STR_TO_JSON(encoded, json_document);

    const auto check_value = T(json_document);

    if (value.hash() != check_value.hash())
    {
        std::cout << "Failed" << std::endl;

        exit(1);
    }
    else
    {
        std::cout << "Passed" << std::endl;
    }
}

template<typename T> static inline void check_binary_serialization(const T &value, const std::string &name = "")
{
    std::cout << "Checking [" << name << "] binary serialization: ";

    const auto encoded = value.serialize();

    const auto check_value = T(encoded);

    if (value.hash() != check_value.hash())
    {
        std::cout << "Failed" << std::endl;

        exit(1);
    }
    else
    {
        std::cout << "Passed" << std::endl;
    }

    check_json_serialization(value, name);
}

int main()
{
    std::cout << "Data Structures Tests" << std::endl << std::endl;

    // Block
    {
        auto structure = Blockchain::block_t();

        check_binary_serialization(structure, "block_t");
    }

    // Genesis Transaction
    {
        auto structure = Blockchain::genesis_transaction_t();

        check_binary_serialization(structure, "genesis_transaction_t");
    }

    // Staker Reward Transaction
    {
        auto structure = Blockchain::staker_reward_transaction_t();

        check_binary_serialization(structure, "staker_reward_transaction_t");
    }

    // Uncommitted Normal Transaction
    {
        auto structure = Blockchain::uncommited_normal_transaction_t();

        check_binary_serialization(structure, "uncommited_normal_transaction_t");
    }

    // Committed Normal Transaction
    {
        auto structure = Blockchain::committed_normal_transaction_t();

        check_binary_serialization(structure, "committed_normal_transaction_t");
    }

    // Uncommitted Stake Transaction
    {
        auto structure = Blockchain::uncommitted_stake_transaction_t();

        check_binary_serialization(structure, "uncommitted_stake_transaction_t");
    }

    // Committed Stake Transaction
    {
        auto structure = Blockchain::committed_stake_transaction_t();

        check_binary_serialization(structure, "committed_stake_transaction_t");
    }

    // Uncommitted Recall Stake Transaction
    {
        auto structure = Blockchain::uncommitted_recall_stake_transaction_t();

        check_binary_serialization(structure, "uncommitted_recall_stake_transaction_t");
    }

    // Committed Recall Stake Transaction
    {
        auto structure = Blockchain::committed_recall_stake_transaction_t();

        check_binary_serialization(structure, "committed_recall_stake_transaction_t");
    }

    // Stake Refund Transaction
    {
        auto structure = Blockchain::stake_refund_transaction_t();

        check_binary_serialization(structure, "stake_refund_transaction_t");
    }

    return 0;
}
