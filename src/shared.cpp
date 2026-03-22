#include "shared.h"

/**
 * @brief Initializes all fields of the SharedData structure.
 *        Sets all counters to zero, initializes mutex locks,
 *        condition variables, and the barrier semaphore.
 *
 * @param shared  pointer to SharedData struct to initialize
 * @param limit   total order production limit set by -n argument
 */
void initSharedData(SharedData* shared, int limit) {

    //  Production limit 
    shared->production_limit = limit;

    //  Reserved queue counters 
    shared->spot_in_queue    = 0;
    shared->market_in_queue  = 0;
    shared->spot_produced    = 0;
    shared->market_produced  = 0;

    //  Proof queue counters 
    shared->proofs_in_queue  = 0;
    shared->proofs_consumed  = 0;

    // Consumer tracking 
    for (int c = 0; c < ExecChainTypeN; c++)
        for (int p = 0; p < OrderTypeN; p++)
            shared->consumed[c][p] = 0;

    // Reserved queue monitor 
    pthread_mutex_init(&shared->reserved_mutex, NULL);
    pthread_cond_init(&shared->not_full_reserved,    NULL);
    pthread_cond_init(&shared->not_full_market_swap, NULL);
    pthread_cond_init(&shared->not_empty_reserved,   NULL);

    //  Proof queue monitor 
    pthread_mutex_init(&shared->proof_mutex, NULL);
    pthread_cond_init(&shared->not_full_proofs,  NULL);
    pthread_cond_init(&shared->not_empty_proofs, NULL);

    //  Barrier semaphore 
    sem_init(&shared->barrier, 0, 0);
}

/**
 * @brief Destroys all synchronization variables in SharedData.
 *        Should be called before program exits.
 *
 * @param shared  pointer to SharedData struct to destroy
 */
void destroySharedData(SharedData* shared) {

    //  Reserved queue monitor 
    pthread_mutex_destroy(&shared->reserved_mutex);
    pthread_cond_destroy(&shared->not_full_reserved);
    pthread_cond_destroy(&shared->not_full_market_swap);
    pthread_cond_destroy(&shared->not_empty_reserved);

    //  Proof queue monitor 
    pthread_mutex_destroy(&shared->proof_mutex);
    pthread_cond_destroy(&shared->not_full_proofs);
    pthread_cond_destroy(&shared->not_empty_proofs);

    //  Barrier semaphore 
    sem_destroy(&shared->barrier);
}
