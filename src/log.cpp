#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "log.h"

/*
 * i/o functions - assumed to be called in a critical section
 */


/* Handle C++ namespaces, ignore if compiled in C 
 * C++ usually uses this #define to declare the C++ standard.
 * It will not be defined if a C compiler is used.
 */
#ifdef __cplusplus
using namespace std;
#endif

sem_t sem_print;  // Semaphore for ensuring mutex printing, 
                  // I know this is bad practice in terms of using Global, which is not ideal.
                  // Do NOT use the Global variables in your code! 

pthread_once_t init_once = PTHREAD_ONCE_INIT;  // Ensures one-time initialization

void initialize_semaphore() {
    sem_init(&sem_print, 0, 1);  // Initialize semaphore with value 1
}

/*
 * Data section - names must align with the enumerated types
 * defined in trade_pipeline.h
 */

/* Names of order producer threads and order types */
const char *order_producers[] = {"SpotLimit order", "MarketSwap order"};
const char *order_producerNames[] = {"SPOT", "SWAP"};

/**
 * Names of order consumer threads (execution threads on two blockchains).
 * Consumers of the orders for executing trades on-chain.
 * Also producers of execution proofs to the settlement thread.
 */
const char *order_consumerNames[] = {"EthExec", "SolExec"};

/**
 * Name of the settlement thread consumer.
 * Consumer of execution proofs, finalizes ledger changes, releases holds,
 * and records final state.
 **/
const char *execution_consumerName = "Settler";

/* double elapsed_s()
 * show seconds of wall clock time used by the process
 */

double elapsed_s() {
  const double ns_per_s = 1e9; /* nanoseconds per second */

  /* Initialize the first time we call this */
  static struct timespec start;
  static int firsttime = 1;

  struct timespec t;

  /* get elapsed wall clock time for this process
   * note:  use CLOCK_PROCESS_CPUTIME_ID for CPU time (not relevant here
   * as we will be sleeping a lot on the condition variables)
   */
  clock_gettime(CLOCK_REALTIME, &t);

  if (firsttime) {
    /* first time we've called the function, store the current
     * time.  This will not track the cost of the first item
     * produced, but is a reasonable approximation for the
     * whole process and avoids having to create an initialization
     * function.
     * (In C++, we'd just build a timer object and pass it around,
     *  but this approximation provides a simple interface for both
     *  C and C++.)
     */
    firsttime = 0;  /* don't do this again */
    start = t;  /* note when we started */
  }
  
  /* determine time delta from start and convert to s */
  double s = (t.tv_sec - start.tv_sec) + 
    (t.tv_nsec - start.tv_nsec) / ns_per_s ;
  return s;
}

/**
 * @brief Show that an order has been added to the reserved-orders queue and 
 *        print the current status of the reserved-orders queue.
 * 
 * @param orderAdded      see header for this type
 * 
 * produced and inQueue reflect numbers *after* adding the current order.
 */
void log_added_order(OrderAdded orderAdded) {
  int idx;
  int total;

  pthread_once(&init_once, initialize_semaphore);  // Ensure one-time initialization

  // lock for mutex printing
  sem_wait(&sem_print);

  /* Show what is in the reserved-orders queue */
  printf("Reserved_orders_queue: ");
  total = 0;  /* total produced */
  for (idx=0; idx < OrderTypeN; idx++) {
    if (idx > 0)
      printf(" + ");  /* separator */
    printf("%d %s", orderAdded.inQueue[idx], order_producerNames[idx]);
    total += orderAdded.inQueue[idx];
  }

  printf(" = %d. ", total);

  printf("Added %s.", order_producers[orderAdded.type]);

  /* Show what has been produced */
  total = 0;
  printf(" Produced: ");
  for (idx=0; idx < OrderTypeN; idx++) {
    total += orderAdded.produced[idx];  /* track total produced */
    if (idx > 0)
      printf(" + ");  /* separator */
    printf("%d %s", orderAdded.produced[idx], order_producerNames[idx]);
  }
  /* total produced over how long */
  printf(" = %d in %.3f s.\n", total, elapsed_s());

  /* This is not really needed, but will be helpful for making sure that you
   * see output prior to a segmentation violation.  This is not usually a
   * good practice as we want to avoid ending the CPU burst prematurely which
   * this will do, but it is a helpful technique.
   */
  fflush(stdout);  

  sem_post(&sem_print); // release the semaphore for printing
};

/**
 * @brief   Show that an order has been removed from the reserved-orders queue 
 *          and print the current status of the reserved-orders queue.
 *
 * @param orderRemoved    see header for this type
 * 
 * Counts reflect numbers *after* the order has been removed
 */
void log_removed_order(OrderRemoved orderRemoved) {
  int idx;
  int total;

  pthread_once(&init_once, initialize_semaphore);  // Ensure one-time initialization

  // lock for mutex printing
  sem_wait(&sem_print);

  /* Show what is in the reserved-orders queue */
  total = 0;
  printf("Reserved_orders_queue: ");
  for (idx=0; idx < OrderTypeN; idx++) {
    if (idx > 0)
      printf(" + ");  /* separator */
    printf("%d %s", orderRemoved.inQueue[idx], order_producerNames[idx]);
    total += orderRemoved.inQueue[idx];
  }
  printf(" = %d. ", total);
  
  /* Show what has been consumed by consumer */
  printf("%s consumed %s.  %s totals: ",
    order_consumerNames[orderRemoved.consumer],
    order_producers[orderRemoved.type],
    order_consumerNames[orderRemoved.consumer]);
  total = 0;
  for (idx = 0; idx < OrderTypeN; idx++) {
    if (idx > 0)
      printf(" + ");  /* separator */
    total += orderRemoved.consumed[idx];  /* track total consumed */
    printf("%d %s", orderRemoved.consumed[idx], order_producerNames[idx]);
  }
  /* total consumed over how long */
  printf(" = %d consumed in %.3f s.\n", total, elapsed_s());

  /* This is not really needed, but will be helpful for making sure that you
   * see output prior to a segmentation violation.  This is not usually a
   * good practice as we want to avoid ending the CPU burst prematurely which
   * this will do, but it is a helpful technique.
   */
  fflush(stdout);

  sem_post(&sem_print); // release the semaphore for printing
};

/**
 * @brief   Show that an item has been added to the execution-proofs queue 
 *          and print the current size of the execution-proofs queue.
 *
 * @param executionAdded       a string combining order consumer name and 
 *                              order name, e.g., EthExecSPOT, see log.c
 * @param executionQueueSize   Size *after* the execution proof has been added
 */
void log_added_execution(const char* executionAdded,
                              unsigned int executionQueueSize) {

  pthread_once(&init_once, initialize_semaphore);  // Ensure one-time initialization

  // lock for mutex printing
  sem_wait(&sem_print);

  /* Show what is in the execution-proofs queue */
  printf("Execution_proofs_queue: produced and added %s in %.3f s, queue size: %d\n", 
         executionAdded, elapsed_s(), executionQueueSize);

  sem_post(&sem_print); // release the semaphore for printing
}

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
                                unsigned int totalSettled) {

  pthread_once(&init_once, initialize_semaphore);  // Ensure one-time initialization

  // lock for mutex printing
  sem_wait(&sem_print);

  /* Show what is in the execution-proofs queue */
  printf("Execution_proofs_queue: removed and settled %s in %.3f s, queue size: %d, total settled: %d\n", 
    executionRemoved, elapsed_s(), executionQueueSize, totalSettled);

  sem_post(&sem_print); // release the semaphore for printing
}

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
void log_order_history(unsigned int produced[], 
                            unsigned int *consumed[]) {
  int p, c;  /* array indices */
  int total;

  printf("\nORDER REPORT\n----------------------------------------\n");
   
  /* show number produced for each order type */
  for (p = 0; p < OrderTypeN; p++) {
    printf("%s producer generated %d orders\n",
	   order_producers[p], produced[p]);
  }
  /* show number consumed by each consumer */
  for (c=0; c < ExecChainTypeN; c++) {
    printf("%s consumed ", order_consumerNames[c]);
    total = 0;
    for (p = 0; p < OrderTypeN; p++) {
      if (p > 0)
	printf(" + ");
    
      total += consumed[c][p];
      printf("%d %s", consumed[c][p], order_producerNames[p]);
    }
    printf(" = %d total\n", total);
  }

  printf("Elapsed time %.3f s\n", elapsed_s());
}
