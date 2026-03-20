#ifndef TRADEPIPELINE_H
#define TRADEPIPELINE_H

/*
 * Arrays with order producer and order consumer names
 * These can be indexed with the enumerated types below
 * and are defined in log.c
 */

 /* Names of order producers (two threads) and order types */
extern const char *order_producers[];
extern const char *order_producerNames[];

/**
 * Names of order consumers (two threads) for executing each order on-chain,
 * that produce execution proofs. So they are also the producers of
 * the execution proofs that will be consumed by the settlement consumer
 * in the next stage.
 * 
 * Consumers of the orders for executing trades on the respective blockchain.
 * Also producers of execution proofs (on-chain transaction records).
 */
extern const char *order_consumerNames[];

/**
 * Name of the consumer of the execution proofs (settlement thread), one thread 
 * that finalizes ledger changes, releases holds, and records final state.
 **/
extern const char *execution_consumerName;

/*
 * Enumerated types to be used by the producers and consumers
 * These are expected in the input/output functions (log.h)
 * should be useful in your producer and consumer code.
 */

/**
 * The reserved-orders queue can hold up to a maximum of 25 unconsumed orders 
 * in its queue at any given time.
 *  
 * There can be no more than 10 MarketSwap orders 
 * in the reserved-orders queue at any given time. 
*/

/*
 * Two producers of orders (threads) are offered: 
 *   one thread for producing SpotLimit orders, 
 *   one thread for producing MarketSwap orders.
 * Each order producer only produces one type of orders,
 * so OrderType is synonymous with the producer type
 */
typedef enum Orders {
  SpotLimit = 0,          // SpotLimit order (e.g., BTC-USD LIMIT BUY 0.25 @ 49,900)
  MarketSwap = 1,         // MarketSwap order (e.g., USDC->ETH MARKET swap $1,000)
  OrderTypeN = 2,         // number of order types
} OrderType;

/* 
 * Two consumers of orders (threads) are executing 
 * the orders on two blockchains.
 * These threads take the orders off from the reserved-orders queue 
 * in the order that they are produced and inserted to the queue (FIFO).
*/
typedef enum ExecChains {
  EthExec = 0,            // Ethereum execution thread (wraps trade as EVM tx)
  SolExec = 1,            // Solana execution thread (wraps trade as Solana tx)
  ExecChainTypeN = 2,     // Number of order consumers / execution producers
} ExecChainType;


typedef struct {
  // What kind of order was produced?
  OrderType type; 
  // Array of number of orders of each type that have been produced
  unsigned int *produced; 
  // Array of number of orders of each type that are
  // in the reserved-orders queue and have not yet been consumed.
  // (inQueue[SpotLimit] and inQueue[MarketSwap])
  unsigned int *inQueue;
} OrderAdded;

typedef struct {
  // Who removed and processed the order?
  ExecChainType consumer; 
  // What kind of order was removed?  As each consumer consumes
  // one type of request, this is the same as the producer.
  OrderType type; 
  // Array of number of orders of each type that have been
  // consumed.
  unsigned int *consumed; 
  // Array of number of orders of each type that are
  // in the reserved-orders queue and have not yet been consumed.
  // (inQueue[SpotLimit] and inQueue[MarketSwap])
  unsigned int *inQueue;
} OrderRemoved;

#endif
