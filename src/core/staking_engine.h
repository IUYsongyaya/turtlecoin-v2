// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef CORE_STAKING_ENGINE_H
#define CORE_STAKING_ENGINE_H

#include <crypto.h>
#include <db_lmdb.h>
#include <errors.h>
#include <types.h>

namespace Core
{
    /**
     * Represents the core staking engine
     */
    class StakingEngine
    {
      public:
        /**
         * Creates a new instance of the Staking Engine with the database in the
         * provided path
         * @param db_path the path in which to store the database
         */
        StakingEngine(const std::string &db_path);

        /**
         * Adds a new candidate to the database
         * @param candidate the candidate public key
         * @return
         */
        Error add_candidate(const Types::Staking::candidate_node_t &candidate);

        /**
         * Adds a new staker to the database
         * @param staker
         * @return
         */
        Error add_staker(const Types::Staking::staker_t &staker);

        /**
         * Calculates the election seed from the given last blocks presented
         * @param last_round_blocks the hashes of the blocks in the last round
         * @return [seed, seed_uint, evenness]
         */
        std::tuple<crypto_public_key_t, uint256_t, bool>
            calculate_election_seed(const std::vector<crypto_hash_t> &last_round_blocks);

        /**
         * Deletes the candidate from the database
         * @param candidate_key
         * @return
         */
        Error delete_candidate(const crypto_public_key_t &candidate_key);

        /**
         * Deletes the staker from the database
         * @param staker_id
         * @return
         */
        Error delete_staker(const crypto_hash_t &staker_id);

        /**
         * Retrieves the candidate record for the given candidate key
         * @param candidate_key
         * @return [found, candidate_record]
         */
        std::tuple<Error, Types::Staking::candidate_node_t> get_candidate(const crypto_public_key_t &candidate_key);

        /**
         * Retrieves all of the active stakes for the given candidate
         * @param candidate_key
         * @return
         */
        std::vector<Types::Staking::stake_t> get_candidate_stakes(const crypto_public_key_t &candidate_key);

        /**
         * Retrieves the number of votes for a specific candidate key
         *
         * Returns 0 if the candidate is unknown
         *
         * @param candidate_key
         * @return
         */
        uint64_t get_candidate_votes(const crypto_public_key_t &candidate_key);

        /**
         * Retrieves the keys for all candidates in the database
         * @return
         */
        std::vector<crypto_public_key_t> get_candidates();

        /**
         * Retrieves the staker record for the given staker key
         * @param staker_key
         * @return [found, staker_record]
         */
        std::tuple<Error, Types::Staking::staker_t> get_staker(const crypto_hash_t &staker_key);

        /**
         * Retrieves all tally of all of a staker's votes for a particular candidate
         * @param staker_id
         * @param candidate_key
         * @return
         */
        uint64_t get_staker_candidate_votes(const crypto_hash_t &staker_id, const crypto_public_key_t &candidate_key);

        /**
         * Retrieves the keys for all stakers in the database
         * @return
         */
        std::vector<crypto_hash_t> get_stakers();

        /**
         * Retrieve all of the stakes that the given staker has placed
         * @param staker_id
         * @return
         */
        std::map<crypto_public_key_t, std::vector<Types::Staking::stake_t>>
            get_staker_stakes(const crypto_hash_t &staker_id);

        /**
         * Recall a the stake with the given parameters
         * @param staker
         * @param stake_txn
         * @param candidate_key
         * @param stake
         * @return
         */
        Error recall_stake(
            const Types::Staking::staker_t &staker,
            const crypto_hash_t &stake_txn,
            const crypto_public_key_t &candidate_key,
            const uint64_t &stake);

        /**
         * Records a stake with the given parameters
         * @param staker
         * @param stake_txn
         * @param candidate_key
         * @param stake
         * @return
         */
        Error record_stake(
            const Types::Staking::staker_t &staker,
            const crypto_hash_t &stake_txn,
            const crypto_public_key_t &candidate_key,
            const uint64_t &stake);

        /**
         * Performs the election process to determine the producers and validators for the next
         * round of blocks given the previous round of block hashes and returns, at maximum, the
         * requested number of elected producers and validators
         * @param last_round_blocks the hashes of the blocks in the last round
         * @param maximum_keys the maximum number of producers and validators to return in each set
         * @return
         */
        std::tuple<std::vector<crypto_public_key_t>, std::vector<crypto_public_key_t>> run_election(
            const std::vector<crypto_hash_t> &last_round_blocks,
            size_t maximum_keys = Configuration::Consensus::ELECTOR_TARGET_COUNT);

      private:
        std::shared_ptr<Database::LMDB> m_db_env;

        std::shared_ptr<Database::LMDBDatabase> m_db_candidates, m_db_stakers, m_db_stakes;

        std::mutex candidates_mutex, stakers_mutex, stakes_mutex;
    };
} // namespace Core

#endif // CORE_STAKING_ENGINE_H
