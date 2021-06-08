// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_CONFIG_H
#define TURTLECOIN_CONFIG_H

#include <crypto.h>
#include <cstdint>

namespace TurtleCoin::Configuration
{
    namespace Version
    {
        const std::string PROJECT_NAME = "TurtleCoin";

        const std::string COPYRIGHT = "Copyright (c) 2021 The TurtleCoin Developers";

        const std::string LICENSE_URL = "https://github.com/turtlecoin/turtlecoin-v2/blob/master/LICENSE";

        const size_t MAJOR = 2;

        const size_t MINOR = 0;

        const size_t PATCH = 0;

        const size_t BUILD = 0;
    } // namespace Version

    const uint64_t GENESIS_BLOCK_TIMESTAMP = 1634788800;

    const uint64_t PUBLIC_ADDRESS_PREFIX = 0x6bb3b1d;

    const size_t MINTED_MONEY_UNLOCK_WINDOW = 60;

    namespace Consensus
    {
        const size_t ELECTOR_TARGET_COUNT = 10;

        /**
         * The minimum percentage of validators in a round that must validate a block for
         * the block to be committed to the chain.
         */
        const size_t VALIDATOR_THRESHOLD = 60;

        /**
         * Permanent candidates injected into the election process so that in the event
         * we are unable to elect enough candidates to support the creation of new blocks.
         * These candidates are inserted into the producer and validator election results
         * regardless of the deterministically random election process. They will always
         * consume a producer and validator slot in every election.
         *
         * Process requires a MINIMUM of THREE (3) for successful launch of the network
         *
         * TODO: These will need updated with real values prior to launch as they are random keys
         *
         */
        const std::vector<crypto_public_key_t> PERMANENT_CANDIDATES = {
            crypto_public_key_t("0dd2ca6545ea58be4a3984c15f14d6451caad4e0d91d2460310c10bd4d0becf7"),
            crypto_public_key_t("775df2eab78f18c9107a6e085a056c055bd515cf1d8746363b4a9c4bfd4951ad"),
            crypto_public_key_t("17cf02ef00953115261750711fe13d2d76d217ca5f54ca175bcecf3b5cc966eb")};
    } // namespace Consensus

    namespace Staking
    {
        const uint64_t CANDIDATE_RECORD_VERSION = 1;

        const uint64_t STAKER_RECORD_VERSION = 1;

        const uint64_t STAKE_RECORD_VERSION = 1;
    } // namespace Staking

    namespace Transaction
    {
        const size_t RING_SIZE = 512; // must be a power of 2

        const size_t MAXIMUM_INPUTS = 8;

        const size_t MINIMUM_OUTPUTS = 2;

        const size_t MAXIMUM_OUTPUTS = 8;

        namespace Types
        {
            const uint64_t GENESIS = 0;

            const uint64_t STAKER_REWARD = 1;

            const uint64_t NORMAL = 2;

            const uint64_t STAKE = 3;

            const uint64_t RECALL_STAKE = 4;

            const uint64_t STAKE_REFUND = 5;
        } // namespace Types

        namespace Fees
        {
            /**
             * The minimum network transaction fee required for all transactions
             */
            const uint64_t MINIMUM_FEE = 1;

            /**
             * How large, in bytes, each chunk of data is
             */
            const uint64_t CHUNK_SIZE = 64;

            /**
             * This allows for a minimal transaction to pass with the MINIMUM_FEE
             */
            const uint64_t BASE_CHUNK_SIZE = CHUNK_SIZE * 4;

            /**
             * This is the amount added for each CHUNK over the BASE_CHUNK_SIZE
             */
            const uint64_t CHUNK_FEE = 1;

            /**
             * The minimum number of difficulty (zeros) required to transmit a transaction
             * on the network
             */
            const size_t MINIMUM_POW_ZEROS = 1;

            /**
             * The maximum number of zeros that will be considered for the PoW discount
             */
            const size_t MAXIMUM_POW_ZEROS = 16;

            /**
             * The multiplier used when calculating the discount for the number of additional PoW zeros
             */
            const float POW_ZERO_DISCOUNT_MULTIPLIER = 2;
        } // namespace Fees

        /**
         * Argon2id parameters for Transaction PoW mining
         */
        namespace ProofOfWork
        {
            const size_t ITERATIONS = 2048;

            const size_t MEMORY = 1024; // 1MB

            const size_t THREADS = 1;
        } // namespace ProofOfWork
    } // namespace Transaction
} // namespace TurtleCoin::Configuration

#endif // TURTLECOIN_CONFIG_H
