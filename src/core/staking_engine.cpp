// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "staking_engine.h"

namespace Core
{
    StakingEngine::StakingEngine(const std::string &db_path)
    {
        m_db_env = Database::LMDB::getInstance(db_path);

        m_db_candidates = m_db_env->open_database("candidates");

        m_db_stakers = m_db_env->open_database("stakers");

        m_db_stakes = m_db_env->open_database("stakes", MDB_CREATE | MDB_DUPSORT);
    }

    Error StakingEngine::add_candidate(const Types::Staking::candidate_node_t &candidate)
    {
        std::scoped_lock lock(candidates_mutex);

        return m_db_candidates->put(candidate.public_signing_key, candidate.serialize());
    }

    Error StakingEngine::add_staker(const Types::Staking::staker_t &staker)
    {
        std::scoped_lock lock(stakers_mutex);

        return m_db_stakers->put(staker.id(), staker.serialize());
    }

    std::tuple<crypto_public_key_t, uint256_t, bool>
        StakingEngine::calculate_election_seed(const std::vector<crypto_hash_t> &last_round_blocks)
    {
        /**
         * First, we take the hashes of every block in the now closed round and calculate
         * the Merkle root for those hashes to establish M as the election seed for the next round
         */
        const auto M = Crypto::Hashing::Merkle::root_hash(last_round_blocks);

        /**
         * Then we take M and convert it to a scalar p via Hs(M) and compute the public key P of p
         * Note: shortcut used as hash_to_point does just that
         */
        const auto P = Crypto::hash_to_point(M);

        // Then we tally the individual bytes of P...
        uint64_t sum = 0;

        for (int i = 0; i < P.size(); ++i)
        {
            sum += P[i];
        }

        // And determine if the result of that is even or odd (returning true if even)
        return {P, P.to_uint256_t(), sum % 2 == 0};
    }

    Error StakingEngine::delete_candidate(const crypto_public_key_t &candidate_key)
    {
        std::scoped_lock lock(candidates_mutex);

        const auto [error, candidate] = get_candidate(candidate_key);

        if (error)
        {
            return {STAKING_CANDIDATE_NOT_FOUND, {}};
        }

        return m_db_candidates->del(candidate_key);
    }

    Error StakingEngine::delete_staker(const crypto_hash_t &staker_id)
    {
        std::scoped_lock lock(stakers_mutex);

        const auto [error, staker] = get_staker(staker_id);

        if (error)
        {
            return {STAKING_STAKER_NOT_FOUND};
        }

        return m_db_stakers->del(staker_id);
    }

    std::tuple<Error, Types::Staking::candidate_node_t>
        StakingEngine::get_candidate(const crypto_public_key_t &candidate_key)
    {
        const auto [error, candidate] =
            m_db_candidates->get<crypto_public_key_t, Types::Staking::candidate_node_t>(candidate_key);

        if (error)
        {
            return {STAKING_CANDIDATE_NOT_FOUND, {}};
        }

        return {error, candidate};
    }

    std::vector<Types::Staking::stake_t> StakingEngine::get_candidate_stakes(const crypto_public_key_t &candidate_key)
    {
        auto txn = m_db_stakes->transaction(true);

        auto cursor = txn->cursor();

        const auto [error, key, stakes] = cursor->get_all<crypto_public_key_t, Types::Staking::stake_t>(candidate_key);

        return stakes;
    }

    uint64_t StakingEngine::get_candidate_votes(const crypto_public_key_t &candidate_key)
    {
        uint64_t votes = 0;

        auto txn = m_db_stakes->transaction(true);

        auto cursor = txn->cursor();

        const auto [error, key, values] = cursor->get_all<crypto_public_key_t, Types::Staking::stake_t>(candidate_key);

        if (!error)
        {
            for (const auto &value : values)
            {
                votes += value.stake;
            }
        }

        return votes;
    }

    std::vector<crypto_public_key_t> StakingEngine::get_candidates()
    {
        return m_db_candidates->list_keys<crypto_public_key_t>();
    }

    std::vector<crypto_hash_t> StakingEngine::get_stakers()
    {
        return m_db_stakers->list_keys<crypto_hash_t>();
    }

    std::tuple<Error, Types::Staking::staker_t> StakingEngine::get_staker(const crypto_hash_t &staker_key)
    {
        const auto [error, staker] = m_db_stakers->get<crypto_hash_t, Types::Staking::staker_t>(staker_key);

        if (error)
        {
            return {Error(STAKING_STAKER_NOT_FOUND), {}};
        }

        return {error, staker};
    }

    uint64_t StakingEngine::get_staker_candidate_votes(
        const crypto_hash_t &staker_id,
        const crypto_public_key_t &candidate_key)
    {
        uint64_t votes = 0;

        const auto staker_stakes = get_staker_stakes(staker_id);

        if (staker_stakes.find(candidate_key) != staker_stakes.end())
        {
            const auto &stakes = staker_stakes.at(candidate_key);

            for (const auto &stake : stakes)
            {
                votes += stake.stake;
            }
        }

        return votes;
    }

    std::map<crypto_public_key_t, std::vector<Types::Staking::stake_t>>
        StakingEngine::get_staker_stakes(const crypto_hash_t &staker_id)
    {
        std::map<crypto_public_key_t, std::vector<Types::Staking::stake_t>> results;

        // get all of the candidate keys
        const auto candidates = m_db_stakes->list_keys<crypto_public_key_t>();

        // loop through the candidate keys
        for (const auto &candidate : candidates)
        {
            // get all of the stakes for the candidate
            const auto stakes = get_candidate_stakes(candidate);

            std::vector<Types::Staking::stake_t> candidate_stakes;

            // loop through the stakes
            for (const auto &stake : stakes)
            {
                // if the staker id of the stake matches what we requested, add it to the results
                if (stake.staker_id == staker_id)
                {
                    candidate_stakes.push_back(stake);
                }
            }

            if (!candidate_stakes.empty())
            {
                results.insert({candidate, candidate_stakes});
            }
        }

        return results;
    }

    Error StakingEngine::recall_stake(
        const Types::Staking::staker_t &staker,
        const crypto_hash_t &stake_txn,
        const crypto_public_key_t &candidate_key,
        const uint64_t &stake)
    {
        std::scoped_lock lock(stakes_mutex);

        // create the stake record
        const auto stake_record = Types::Staking::stake_t(staker.id(), stake_txn, stake);

        // now delete it
        return m_db_stakes->del(candidate_key, stake_record.serialize());
    }

    Error StakingEngine::record_stake(
        const Types::Staking::staker_t &staker,
        const crypto_hash_t &stake_txn,
        const crypto_public_key_t &candidate_key,
        const uint64_t &stake)
    {
        std::scoped_lock lock(stakes_mutex);

        {
            // check to see if the candidate exists
            const auto [error, value] = get_candidate(candidate_key);

            // can't stake a candidate that does not exist
            if (error)
            {
                return Error(STAKING_CANDIDATE_NOT_FOUND);
            }
        }

        {
            // add the staker to the database
            auto error = add_staker(staker);

            if (error)
            {
                return error;
            }
        }

        // create the stake record
        const auto stake_record = Types::Staking::stake_t(staker.id(), stake_txn, stake);

        {
            const auto [error, value] = get_staker(staker.id());

            if (error)
            {
                return error;
            }
        }

        // attempt to write the stake to the database
        return m_db_stakes->put(candidate_key, stake_record.serialize());
    }

    std::tuple<std::vector<crypto_public_key_t>, std::vector<crypto_public_key_t>>
        StakingEngine::run_election(const std::vector<crypto_hash_t> &last_round_blocks, size_t maximum_keys)
    {
        // Fetch all of the candidates public keys so we can do some electing
        const auto candidates = get_candidates();

        // Fetch the round seed
        const auto [P, P_val, P_even] = calculate_election_seed(last_round_blocks);

        // set up our upper and lower houses (producers & validators)
        std::map<uint256_t, crypto_public_key_t> upper_house, lower_house;

        // which house is which is based on the P evenness
        auto &producer_candidates = (P_even) ? lower_house : upper_house;

        auto &validator_candidates = (P_even) ? upper_house : lower_house;

        // Loop through all of the candidates to figure out what house they go into
        for (const auto &candidate : candidates)
        {
            const auto votes = get_candidate_votes(candidate);

            // Candidates with no votes don't get to come to the party
            if (votes == 0)
            {
                continue;
            }

            // If the candidate is less than P, it goes in the lower house; otherwise, in the upper house
            auto &target_house = (candidate <= P) ? lower_house : upper_house;

            /**
             * If another candidate is already in the house with the same number of
             * votes as this candidate, then we need to determine who gets bumped
             * out of the running. To do so, we run a mini election process between
             * the candidates by hashing their public keys and performing "normal"
             * election logic to determine who wins the candidate seat
             */
            if (target_house.find(votes) != target_house.end())
            {
                std::map<uint256_t, crypto_public_key_t> district_house;

                district_house.insert({Crypto::Hashing::sha3(candidate).to_uint256_t(), candidate});

                district_house.insert({Crypto::Hashing::sha3(target_house.at(votes)).to_uint256_t(), candidate});

                const auto last = std::prev(district_house.end());

                /**
                 * Take the uint256_t value of the round seed and modulo it using the
                 * highest uint256_t value of the hash of the candidate public keys
                 */
                const auto e = P_val % last->first;

                /**
                 * Returns an iterator pointing to the first element that is greater than e
                 */
                const auto target_house_winner = validator_candidates.upper_bound(e);

                target_house.at(votes) = target_house_winner->second;

                continue;
            }

            target_house.insert({votes, candidate});
        }

        // strip off the bottoms
        if (!lower_house.empty())
        {
            lower_house.erase(lower_house.begin());
        }

        if (!upper_house.empty())
        {
            upper_house.erase(upper_house.begin());
        }

        // strip off the tops
        if (!lower_house.empty())
        {
            lower_house.erase(std::prev(lower_house.end()));
        }

        if (!upper_house.empty())
        {
            upper_house.erase(std::prev(upper_house.end()));
        }

        // Set up our final results including our permanent candidate members
        std::vector<crypto_public_key_t> producers(
            Configuration::Consensus::PERMANENT_CANDIDATES.begin(),
            Configuration::Consensus::PERMANENT_CANDIDATES.end()),
            validators(
                Configuration::Consensus::PERMANENT_CANDIDATES.begin(),
                Configuration::Consensus::PERMANENT_CANDIDATES.end());

        // try to fill the producers vector with the necessary keys
        while (!producer_candidates.empty() && producers.size() < maximum_keys)
        {
            const auto last = std::prev(producer_candidates.end());

            /**
             * Select the current elector spot using the P_value modulo the maximum
             * votes of the candidate thus establishing the e point
             */
            const auto e = P_val % last->first;

            /**
             * Returns an iterator pointing to the first element that is greater than e
             */
            const auto elected = producer_candidates.upper_bound(e);

            // congrats! this candidate was elected
            producers.push_back(elected->second);

            // once you're elected, you can't be elected again
            producer_candidates.erase(elected);
        }

        // try to fill the validators vector with the necessary keys
        while (!validator_candidates.empty() && validators.size() < maximum_keys)
        {
            const auto last = std::prev(validator_candidates.end());

            /**
             * Select the current elector spot using the P_value modulo the maximum
             * votes of the candidate thus establishing the e point
             */
            const auto e = P_val % last->first;

            /**
             * Returns an iterator pointing to the first element that is greater than e
             */
            const auto elected = validator_candidates.upper_bound(e);

            /**
             * If, somehow we find a candidate in our validator candidates that has already
             * been elected as a producer, then we remove them from the vector of validator
             * candidates.
             */
            if (std::find(producers.begin(), producers.end(), elected->second) != producers.end())
            {
                validator_candidates.erase(elected);

                continue;
            }

            // congrats! this candidate was elected
            validators.push_back(elected->second);

            // once you're elected, you can't be elected again
            validator_candidates.erase(elected);
        }

        // sort the vectors to establish the order
        std::sort(producers.begin(), producers.end());

        std::sort(validators.begin(), validators.end());

        return {producers, validators};
    }
} // namespace Core
