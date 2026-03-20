#ifndef SHARED_H
#define SHARED_H

#include <queue>
#include <pthread.h>
#include <semaphore.h>
#include "trade_pipeline.h"

// ----- Constants -----
#define MAX_RESERVED_ORDERS  25
#define MAX_MARKET_SWAP      10
#define MAX_EXEC_PROOFS      15

// ----- Order Struct -----
struct Order {
    OrderType type;
};

// ----- Execution Proof -----
struct ExecutionProof {
    OrderType      type;
    ExecChainType  producer;   // EthExec or SolExec
};

// ----- Shared Data -----
struct SharedData {
    // ===== Stage 1 → Stage 2: Reserved Orders Queue =====
    std::queue<Order> reservedQueue;
    int spot_in_queue;            // current SPOT count in queue
    int market_in_queue;          // current SWAP count in queue
    int spot_produced;            // total SPOT produced so far
    int market_produced;          // total SWAP produced so far
    int production_limit;         // set by -n argument

    // ===== Stage 2 → Stage 3: Execution Proofs Queue =====
    std::queue<ExecutionProof> proofQueue;
    int proofs_in_queue;          // current size of proof queue
    int proofs_consumed;          // total settled so far

    // ===== Per-consumer tracking (for log_order_history) =====
    unsigned int consumed[ExecChainTypeN][OrderTypeN];

    // ===== Synchronization: Reserved Queue Monitor =====
    pthread_mutex_t reserved_mutex;
    pthread_cond_t  not_full_reserved;      // queue >= 25
    pthread_cond_t  not_full_market_swap;   // SWAP in queue >= 10
    pthread_cond_t  not_empty_reserved;     // queue == 0

    // ===== Synchronization: Proof Queue Monitor =====
    pthread_mutex_t proof_mutex;
    pthread_cond_t  not_full_proofs;        // queue >= 15
    pthread_cond_t  not_empty_proofs;       // queue == 0

    // ===== Barrier: main thread waits for Settler =====
    sem_t barrier;
};

/**
 * @brief Initializes all fields of the SharedData structure.
 *        Sets all counters to zero, initializes mutex locks,
 *        condition variables, and the barrier semaphore.
 *
 * @param shared  pointer to SharedData struct to initialize
 * @param limit   total order production limit set by -n argument
 */
void initSharedData(SharedData* shared, int limit);

/**
 * @brief Destroys all synchronization variables in SharedData.
 *        Should be called before program exits.
 *
 * @param shared  pointer to SharedData struct to destroy
 */
void destroySharedData(SharedData* shared);

#endif
