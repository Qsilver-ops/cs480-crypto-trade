#ifndef CONSUMER_H
#define CONSUMER_H

#include "shared.h"

// ----- Stage 2: Order Consumer / Execution Producer -----
struct ConsumerArgs {
    SharedData*   shared;
    ExecChainType chain;    // EthExec or SolExec
    int           sleep_ms;
};

/**
 * @brief Order consumer / execution-proof producer thread function.
 *        Removes orders from the reserved-orders queue (FIFO), executes
 *        them on the respective blockchain, and adds execution proofs
 *        to the execution-proofs queue.
 *
 * @param arg  pointer to ConsumerArgs struct containing shared data,
 *             chain type (EthExec or SolExec), and sleep time
 * @return NULL
 */
void* cryptoSLorMS_cons(void* arg);

#endif
