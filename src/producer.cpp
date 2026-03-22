#include <unistd.h>
#include <pthread.h>
#include "producer.h"
#include "log.h"

/**
 * @brief Producer thread function for SpotLimit or MarketSwap orders.
 *        Produces orders and adds them to the reserved-orders queue
 *        until the production limit is reached.
 *        Sleeps outside the critical section to simulate production time.
 *
 * @param arg  pointer to ProducerArgs struct containing shared data,
 *             order type, and sleep time
 * @return NULL
 */
void* cryptoSLorMS_prod(void* arg) {
    ProducerArgs* args   = (ProducerArgs*) arg;
    SharedData*   shared = args->shared;
    OrderType     myType = args->type;

    while (true) {

        //  Simulate production OUTSIDE critical section 
        if (args->sleep_ms > 0)
            usleep(args->sleep_ms * 1000);

        //  Enter critical section 
        pthread_mutex_lock(&shared->reserved_mutex);

        // Check if production limit already reached 
        int total_produced = shared->spot_produced + shared->market_produced;
        if (total_produced >= shared->production_limit) {
            pthread_mutex_unlock(&shared->reserved_mutex);
            break;
        }

        //  Wait if reserved queue is full (max 25) 
        while ((int)shared->reservedQueue.size() >= MAX_RESERVED_ORDERS) {
            pthread_cond_wait(&shared->not_full_reserved,
                              &shared->reserved_mutex);
            // Re-check limit after waking
            total_produced = shared->spot_produced + shared->market_produced;
            if (total_produced >= shared->production_limit) {
                pthread_mutex_unlock(&shared->reserved_mutex);
                return NULL;
            }
        }

        //  If MarketSwap: also wait if SWAP limit reached (max 10) 
        if (myType == MarketSwap) {
            while (shared->market_in_queue >= MAX_MARKET_SWAP) {
                pthread_cond_wait(&shared->not_full_market_swap,
                                  &shared->reserved_mutex);
                // Re-check limit after waking
                total_produced = shared->spot_produced + shared->market_produced;
                if (total_produced >= shared->production_limit) {
                    pthread_mutex_unlock(&shared->reserved_mutex);
                    return NULL;
                }
            }
        }

        //  Add order to queue 
        Order newOrder;
        newOrder.type = myType;
        shared->reservedQueue.push(newOrder);

        //  Update counters 
        if (myType == SpotLimit) {
            shared->spot_produced++;
            shared->spot_in_queue++;
        } else {
            shared->market_produced++;
            shared->market_in_queue++;
        }

        // Build OrderAdded for logging (INSIDE critical section) 
        unsigned int inQueue[OrderTypeN];
        inQueue[SpotLimit]  = (unsigned int)shared->spot_in_queue;
        inQueue[MarketSwap] = (unsigned int)shared->market_in_queue;

        unsigned int produced[OrderTypeN];
        produced[SpotLimit]  = (unsigned int)shared->spot_produced;
        produced[MarketSwap] = (unsigned int)shared->market_produced;

        OrderAdded orderAdded;
        orderAdded.type     = myType;
        orderAdded.inQueue  = inQueue;
        orderAdded.produced = produced;

        // --- Log INSIDE critical section per assignment requirement ---
        log_added_order(orderAdded);

        // --- Signal consumers that queue is no longer empty ---
        pthread_cond_signal(&shared->not_empty_reserved);

        // --- Exit critical section ---
        pthread_mutex_unlock(&shared->reserved_mutex);
    }

    return NULL;
}
