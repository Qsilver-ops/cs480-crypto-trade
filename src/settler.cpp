#include <unistd.h>
#include <pthread.h>
#include <string>
#include "settler.h"
#include "log.h"

/**
 * @brief Settlement consumer thread function.
 *        Removes execution proofs from the execution-proofs queue (FIFO),
 *        finalizes ledger changes, releases holds, and records final state.
 *        Signals the main thread barrier when all proofs are settled.
 *
 * @param arg  pointer to SettlerArgs struct containing shared data
 *             and sleep time
 * @return NULL
 */
void* cryptoexecutproofsettle_cons(void* arg) {
    SettlerArgs* args   = (SettlerArgs*) arg;
    SharedData*  shared = args->shared;

    while (true) {

        // --- Enter proof queue critical section ---
        pthread_mutex_lock(&shared->proof_mutex);

        // --- Wait if proof queue is empty ---
        while (shared->proofQueue.empty()) {
            // Check if all proofs have been consumed
            if (shared->proofs_consumed >= shared->production_limit) {
                pthread_mutex_unlock(&shared->proof_mutex);
                sem_post(&shared->barrier);
                return NULL;
            }
            pthread_cond_wait(&shared->not_empty_proofs,
                              &shared->proof_mutex);
        }

        // --- Remove execution proof from queue (FIFO) ---
        ExecutionProof proof = shared->proofQueue.front();
        shared->proofQueue.pop();
        shared->proofs_in_queue--;
        shared->proofs_consumed++;

        // --- Build proof string e.g. "EthExecSPOT" ---
        std::string proofStr = std::string(order_consumerNames[proof.producer])
                             + std::string(order_producerNames[proof.type]);

        // --- Log INSIDE critical section per assignment requirement ---
        log_removed_execution(proofStr.c_str(),
                              (unsigned int)shared->proofs_in_queue,
                              (unsigned int)shared->proofs_consumed);

        // --- Signal consumers that proof queue has space ---
        pthread_cond_signal(&shared->not_full_proofs);

        // --- Exit proof queue critical section ---
        pthread_mutex_unlock(&shared->proof_mutex);

        // --- Simulate settlement OUTSIDE critical section ---
        if (args->sleep_ms > 0)
            usleep(args->sleep_ms * 1000);

        // --- Check if all proofs settled, signal main thread barrier ---
        pthread_mutex_lock(&shared->proof_mutex);
        if (shared->proofs_consumed >= shared->production_limit &&
            shared->proofQueue.empty()) {
            pthread_mutex_unlock(&shared->proof_mutex);
            sem_post(&shared->barrier);
            return NULL;
        }
        pthread_mutex_unlock(&shared->proof_mutex);
    }

    return NULL;
}
