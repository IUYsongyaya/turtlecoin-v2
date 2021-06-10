// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cli_header.h>
#include <types.h>

using namespace Types;

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

template<typename T> static inline void check_binary_serialization_by_hash(const T &value, const std::string &name = "")
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

template<typename T> static inline void check_binary_serialization(const T &value, const std::string &name = "")
{
    std::cout << "Checking [" << name << "] binary serialization: ";

    const auto encoded = value.serialize();

    const auto check_value = T(encoded);

    if (value.to_string() != check_value.to_string())
    {
        std::cout << "Failed" << std::endl;

        exit(1);
    }
    else
    {
        std::cout << "Passed" << std::endl;
    }
}

int main()
{
    print_cli_header();

    std::cout << "Data Structures Tests" << std::endl << std::endl;

    // Block w/ Genesis
    {
        auto structure = Blockchain::block_t();

        check_binary_serialization_by_hash(structure, "block_t[genesis]");
    }

    // Block w/ Staker Reward
    {
        auto structure = Blockchain::block_t();

        structure.reward_tx = Blockchain::staker_reward_transaction_t();

        check_binary_serialization_by_hash(structure, "block_t[staker_reward]");
    }

    // Genesis Transaction
    {
        auto structure = Blockchain::genesis_transaction_t();

        check_binary_serialization_by_hash(structure, "genesis_transaction_t");
    }

    // Staker Reward Transaction
    {
        auto structure = Blockchain::staker_reward_transaction_t();

        check_binary_serialization_by_hash(structure, "staker_reward_transaction_t");
    }

    // Uncommitted Normal Transaction
    {
        auto structure = Blockchain::uncommited_normal_transaction_t();

        check_binary_serialization_by_hash(structure, "uncommited_normal_transaction_t");
    }

    // Committed Normal Transaction
    {
        auto structure = Blockchain::committed_normal_transaction_t();

        check_binary_serialization_by_hash(structure, "committed_normal_transaction_t");
    }

    // Uncommitted Stake Transaction
    {
        auto structure = Blockchain::uncommitted_stake_transaction_t();

        check_binary_serialization_by_hash(structure, "uncommitted_stake_transaction_t");
    }

    // Committed Stake Transaction
    {
        auto structure = Blockchain::committed_stake_transaction_t();

        check_binary_serialization_by_hash(structure, "committed_stake_transaction_t");
    }

    // Uncommitted Recall Stake Transaction
    {
        auto structure = Blockchain::uncommitted_recall_stake_transaction_t();

        check_binary_serialization_by_hash(structure, "uncommitted_recall_stake_transaction_t");
    }

    // Committed Recall Stake Transaction
    {
        auto structure = Blockchain::committed_recall_stake_transaction_t();

        check_binary_serialization_by_hash(structure, "committed_recall_stake_transaction_t");
    }

    // Stake Refund Transaction
    {
        auto structure = Blockchain::stake_refund_transaction_t();

        check_binary_serialization_by_hash(structure, "stake_refund_transaction_t");
    }

    // Candidate
    {
        auto structure = Staking::candidate_node_t();

        check_binary_serialization(structure, "candidate_node_t");
    }

    // Staker
    {
        auto structure = Staking::staker_t();

        check_binary_serialization(structure, "staker_t");
    }

    // Stake
    {
        auto structure = Staking::stake_t();

        check_binary_serialization(structure, "stake_t");
    }

    // Data Packet
    {
        auto structure = Network::packet_data_t();

        check_binary_serialization(structure, "packet_data_t");
    }

    // Handshake Packet
    {
        auto structure = Network::packet_handshake_t();

        check_binary_serialization(structure, "packet_handshake_t");
    }

    // Keepalive Packet
    {
        auto structure = Network::packet_keepalive_t();

        check_binary_serialization(structure, "packet_keepalive_t");
    }

    // Peer Exchange Packet
    {
        auto structure = Network::packet_peer_exchange_t();

        check_binary_serialization(structure, "packet_peer_exchange_t");
    }

    // Network Peer
    {
        auto structure = Network::network_peer_t();

        check_binary_serialization(structure, "network_peer_t");
    }

    std::cout << std::endl;

    return 0;
}
