#ifndef LOG_H
#define LOG_H

#include "trade_pipeline.h"

/**
 * @brief Show that an order has been added to the reserved-orders queue and 
 *        print the current status of the reserved-orders queue.
 * 
 * @param orderAdded      see header for this type
 * 
 * produced and inQueue reflect numbers *after* adding the current order.
 */
void log_added_order (OrderAdded orderAdded);

/**
 * @brief   Show that an order has been removed from the reserved-orders queue 
 *          and print the current status of the reserved-orders queue.
 *
 * @param orderRemoved    see header for this type
 * 
 * Counts reflect numbers *after* the order has been removed
 */
void log_removed_order(OrderRemoved orderRemoved);

/**
 * @brief   Show that an item has been added to the execution-proofs queue 
 *          and print the current size of the execution-proofs queue.
 *
 * @param executionAdded       a string combining order consumer name and 
 *                              order name, e.g., EthExecSPOT, see log.c
 * @param executionQueueSize   Size *after* the execution proof has been added
 */
void log_added_execution(const char* executionAdded,
                              unsigned int executionQueueSize);

/**
 * @brief   Show that an item has been removed from the execution-proofs queue 
 *          and print the current size of the execution-proofs queue.
 *
 * @param executionRemoved     a string combining order consumer name and 
 *                              order name, e.g., EthExecSPOT, see log.c
 * @param executionQueueSize   Size *after* the execution proof has been removed
 * @param totalSettled         total number of settled execution proofs so far
 */
void log_removed_execution(const char* executionRemoved,
                                unsigned int executionQueueSize,
                                unsigned int totalSettled);

/**
 * @brief   Show how many orders of each type produced.  
 *          Show how many orders consumed by each consumer.
 * 
 * @param produced   count for each OrderType produced
 * @param consumed   array of pointers to consumed arrays for each consumer
 *                   e.g. consumed[EthExec] points to an array that 
 *                   is indexed by order type
 *                   (it is a 2-D array, consumed[EthExec][SpotLimit] is 
 *                    the number for SpotLimit orders that were 
 *                    consumed by EthExec)
 */
void log_order_history(unsigned int produced[], unsigned int *consumed[]);

#endif
