#ifndef PRODUCER_H
#define PRODUCER_H

#include "shared.h"

struct ProducerArgs {
    SharedData* shared;
    OrderType type;
    int sleep_ms;
};

/**
 * @brief Producer thread function for SpotLimit or MarketSwap orders.
 *        Produces orders and adds them to the reserved-orders queue
 *        until the production limit is reached.
 *
 * @param arg  pointer to ProducerArgs struct containing shared data,
 *             order type, and sleep time
 * @return NULL
 */
void* cryptoSLorMS_prod(void* arg);

#endif
