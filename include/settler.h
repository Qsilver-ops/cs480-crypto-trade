#ifndef SETTLER_H
#define SETTLER_H

#include "shared.h"

// ----- Stage 3: Settlement Consumer -----
struct SettlerArgs {
    SharedData* shared;
    int         sleep_ms;
};

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
void* cryptoexecutproofsettle_cons(void* arg);

#endif
