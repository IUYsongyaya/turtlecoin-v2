// Copyright (c) 2021, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#ifndef TURTLECOIN_CONFIG_H
#define TURTLECOIN_CONFIG_H

#include <crypto.h>
#include <cstdint>

struct SeedNode
{
    SeedNode(std::string host, const uint16_t &port): host(std::move(host)), port(port) {}

    std::string host;
    uint16_t port;
};

namespace Configuration
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

    /**
     * The default logging level for all applications in this project
     */
    const size_t DEFAULT_LOG_LEVEL = 4;

    /**
     * Defines how long the threads sleep between polling intervals
     * the longer the interval, the slower data may be processed
     *
     * NOTE: This value is expressed in milliseconds
     */
    const int THREAD_POLLING_INTERVAL = 50; //

    /**
     * Defines how long we should wait for outbound connection attempts to
     * complete whether they be via HTTP, ZMQ, or etc.
     *
     * NOTE: This value is expressed in milliseconds
     */
    const int DEFAULT_CONNECTION_TIMEOUT = 2'000;

    /**
     * This is the GENESIS block creation timestamp (seconds since UNIX epoch)
     */
    const uint64_t GENESIS_BLOCK_TIMESTAMP = 1634788800;

    /**
     * Our public address prefix
     */
    const uint64_t PUBLIC_ADDRESS_PREFIX = 0x6bb3b1d;

    const std::string DEFAULT_DATA_DIR = "." + Version::PROJECT_NAME;

    namespace ZMQ
    {
        /**
         * The following keys must be set manually as all clients must have the server's
         * public key to make a valid connection.
         * See http://rfc.zeromq.org/spec:32 for more information on how the keys are encoded
         */
        const std::string SERVER_SECRET_KEY = "!EGQIc+DG97q$Y4DOY}.[8l!%dVf*-W{S.^.Gy&z";
    } // namespace ZMQ

    namespace Notifier
    {
        const uint16_t DEFAULT_BIND_PORT = 12899;
    }

    namespace P2P
    {
        /**
         * Our current P2P version number
         */
        const uint16_t VERSION = 1;

        /**
         * The minimum P2P version that we can/will talk to
         */
        const uint16_t MINIMUM_VERSION = 1;

        /**
         * Defines how often we send a keep alive packet on the P2P network
         */
        const size_t KEEPALIVE_INTERVAL = 30'000; // expressed in milliseconds

        /**
         * Defines how often we send a peer exchange packet on the P2P network
         * to discover new peers
         */
        const size_t PEER_EXCHANGE_INTERVAL = 120'000; // expressed in milliseconds

        /**
         * Defines how often we check our current outgoing connection count
         * and attempt new connections to make up the difference
         */
        const size_t CONNECTION_MANAGER_INTERVAL = 30'000; // expressed in milliseconds

        /**
         * The maximum number of peers that we will send (or accept) in a handshake
         * or peer exchange packet, if more than this are received, the packet is
         * discarded as a protocol violation error
         */
        const size_t MAXIMUM_PEERS_EXCHANGED = 200;

        /**
         * Peers in our peer database will be pruned from our database if the last
         * seen time exceeds this value
         */
        const uint64_t PEER_PRUNE_TIME = 86'400; // 1 day

        /**
         * Defines the default bind port for listening for P2P connections
         */
        const uint16_t DEFAULT_BIND_PORT = 12897;

        /**
         * Defines the list of P2P bootstrap/seed nodes for which we will attempt
         * to connect to if our peer list database is empty
         */
        const std::vector<SeedNode> SEED_NODES = {{"165.227.252.132", 12897}};

        /**
         * Sets the default outbound connection count that we will try to maintain
         */
        const size_t DEFAULT_CONNECTION_COUNT = SEED_NODES.size() + 8;
    } // namespace P2P

    namespace API
    {
        /**
         * Defines the default node bind port for listening for HTTP requests
         */
        const uint16_t DEFAULT_NODE_BIND_PORT = 12898;

        /**
         * Defines the default wallet bind port for listening for HTTP requests
         */
        const uint16_t DEFAULT_WALLET_BIND_PORT = 18070;
    } // namespace API

    namespace Consensus
    {
        /**
         * This defines the target number of electors we will select for each round as
         * producers and validators
         */
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
        /**
         * Defines the number of ring participants required when signing a
         * transaction input
         *
         * NOTE: Must be a power of two (2)
         */
        const size_t RING_SIZE = 512;

        /**
         * The maximum number of inputs permitted in a single transaction
         */
        const size_t MAXIMUM_INPUTS = 8;

        /**
         * The minimum number of outputs required in a single transaction
         */
        const size_t MINIMUM_OUTPUTS = 2;

        /**
         * The maximum number of outputs permitted in a single transaction
         */
        const size_t MAXIMUM_OUTPUTS = 8;

        /**
         * The maximum amount of data that can be stored in the extra field
         * of a normal transaction
         */
        const size_t MAXIMUM_EXTRA_SIZE = 1'024;

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
            const size_t ITERATIONS = 2'048;

            const size_t MEMORY = 1'024; // expressed in kilobytes (1MB)

            const size_t THREADS = 1;
        } // namespace ProofOfWork
    } // namespace Transaction
} // namespace Configuration

#endif // TURTLECOIN_CONFIG_H
