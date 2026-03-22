#include <unistd.h>
#include <pthread.h>
#include <string>
#include "consumer.h"
#include "log.h"

/**
 * @brief Order consumer / execution-proof producer thread function.
 *        Removes orders from the reserved-orders queue (FIFO), executes
 *        them on the respective blockchain, and adds execution proofs
 *        to the execution-proofs queue.
 *        Sleeps outside critical section to simulate execution time.
 *
 * @param arg  pointer to ConsumerArgs struct containing shared data,
 *             chain type (EthExec or SolExec), and sleep time
 * @return NULL
 */
void* cryptoSLorMS_cons(void* arg) {
    ConsumerArgs* args   = (ConsumerArgs*) arg;
    SharedData*   shared = args->shared;
    ExecChainType myChain = args->chain;

    while (true) {

        // Enter reserved queue critical section 
        pthread_mutex_lock(&shared->reserved_mutex);

        //  Wait if reserved queue is empty 
        while (shared->reservedQueue.empty()) {
            // Check if production is done and queue is empty
            int total_produced = shared->spot_produced + shared->market_produced;
            if (total_produced >= shared->production_limit) {
                pthread_mutex_unlock(&shared->reserved_mutex);
                return NULL;
            }
            pthread_cond_wait(&shared->not_empty_reserved,
                              &shared->reserved_mutex);
        }

        //  Check again after waking if we should exit 
        int total_produced = shared->spot_produced + shared->market_produced;
        if (shared->reservedQueue.empty() &&
            total_produced >= shared->production_limit) {
            pthread_mutex_unlock(&shared->reserved_mutex);
            return NULL;
        }

        //  Remove order from reserved queue (FIFO)
        Order order = shared->reservedQueue.front();
        shared->reservedQueue.pop();

        //  Update reserved queue counters 
        if (order.type == SpotLimit)
            shared->spot_in_queue--;
        else
            shared->market_in_queue--;

        //  Update consumer tracking 
        shared->consumed[myChain][order.type]++;

        // Build OrderRemoved for logging (INSIDE critical section) 
        unsigned int inQueue[OrderTypeN];
        inQueue[SpotLimit]  = (unsigned int)shared->spot_in_queue;
        inQueue[MarketSwap] = (unsigned int)shared->market_in_queue;

        unsigned int consumed[OrderTypeN];
        consumed[SpotLimit]  = shared->consumed[myChain][SpotLimit];
        consumed[MarketSwap] = shared->consumed[myChain][MarketSwap];

        OrderRemoved orderRemoved;
        orderRemoved.consumer = myChain;
        orderRemoved.type     = order.type;
        orderRemoved.inQueue  = inQueue;
        orderRemoved.consumed = consumed;

        //  Log INSIDE critical section per assignment requirement
        log_removed_order(orderRemoved);

        //  Signal producers that space is available 
        pthread_cond_signal(&shared->not_full_reserved);

        //  Signal MarketSwap producer if a swap was removed 
        if (order.type == MarketSwap)
            pthread_cond_signal(&shared->not_full_market_swap);

        // Exit reserved queue critical section 
        pthread_mutex_unlock(&shared->reserved_mutex);

        //  Simulate execution OUTSIDE critical section 
        if (args->sleep_ms > 0)
            usleep(args->sleep_ms * 1000);

        //  Build execution proof string e.g. "EthExecSPOT"
        std::string proofStr = std::string(order_consumerNames[myChain])
                             + std::string(order_producerNames[order.type]);

        // Enter proof queue critical section 
        pthread_mutex_lock(&shared->proof_mutex);

        //  Wait if proof queue is full (max 15) 
        while (shared->proofs_in_queue >= MAX_EXEC_PROOFS) {
            pthread_cond_wait(&shared->not_full_proofs,
                              &shared->proof_mutex);
        }

        // Add execution proof to queue 
        ExecutionProof proof;
        proof.type     = order.type;
        proof.producer = myChain;
        shared->proofQueue.push(proof);
        shared->proofs_in_queue++;

        //  Log INSIDE critical section per assignment requirement 
        log_added_execution(proofStr.c_str(),
                            (unsigned int)shared->proofs_in_queue);

        //  Signal settler that proof is available 
        pthread_cond_signal(&shared->not_empty_proofs);

        // Exit proof queue critical section 
        pthread_mutex_unlock(&shared->proof_mutex);
    }

    return NULL;
}
