#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "shared.h"
#include "producer.h"
#include "consumer.h"
#include "settler.h"
#include "log.h"
/**
 * @brief Parses command line arguments, initializes shared data,
 *        creates all producer and consumer threads, and waits for
 *        settlement to complete before exiting.
 *
 * @param argc  number of command line arguments
 * @param argv  array of command line argument strings
 * @return 0 on normal exit
 */
int main(int argc, char* argv[]) {

    // ----- Default argument values -----
    int n = 120;   // total orders to produce
    int s = 0;     // SpotLimit sleep ms
    int w = 0;     // MarketSwap sleep ms
    int e = 0;     // EthExec sleep ms
    int l = 0;     // SolExec sleep ms
    int t = 0;     // Settler sleep ms

    // ----- Parse command line arguments -----
    int option;
    while ((option = getopt(argc, argv, "n:s:w:e:l:t:")) != -1) {
        switch (option) {
            case 'n': n = atoi(optarg); break;
            case 's': s = atoi(optarg); break;
            case 'w': w = atoi(optarg); break;
            case 'e': e = atoi(optarg); break;
            case 'l': l = atoi(optarg); break;
            case 't': t = atoi(optarg); break;
            default:
                fprintf(stderr, "Usage: %s [-n N] [-s N] [-w N] [-e N] [-l N] [-t N]\n",
                        argv[0]);
                exit(1);
        }
    }

    // ----- Initialize shared data -----
    SharedData shared;
    initSharedData(&shared, n);

    // ----- Set up producer args -----
    ProducerArgs spotArgs;
    spotArgs.shared   = &shared;
    spotArgs.type     = SpotLimit;
    spotArgs.sleep_ms = s;

    ProducerArgs swapArgs;
    swapArgs.shared   = &shared;
    swapArgs.type     = MarketSwap;
    swapArgs.sleep_ms = w;

    // ----- Set up consumer args -----
    ConsumerArgs ethArgs;
    ethArgs.shared   = &shared;
    ethArgs.chain    = EthExec;
    ethArgs.sleep_ms = e;

    ConsumerArgs solArgs;
    solArgs.shared   = &shared;
    solArgs.chain    = SolExec;
    solArgs.sleep_ms = l;

    // ----- Set up settler args -----
    SettlerArgs settlerArgs;
    settlerArgs.shared   = &shared;
    settlerArgs.sleep_ms = t;

    // ----- Create all threads at the same time -----
    pthread_t spotThread, swapThread;
    pthread_t ethThread,  solThread;
    pthread_t settlerThread;

    pthread_create(&spotThread,    NULL, cryptoSLorMS_prod,           &spotArgs);
    pthread_create(&swapThread,    NULL, cryptoSLorMS_prod,           &swapArgs);
    pthread_create(&ethThread,     NULL, cryptoSLorMS_cons,           &ethArgs);
    pthread_create(&solThread,     NULL, cryptoSLorMS_cons,           &solArgs);
    pthread_create(&settlerThread, NULL, cryptoexecutproofsettle_cons, &settlerArgs);

    // ----- Main thread waits for settler to finish -----
    sem_wait(&shared.barrier);

    // ----- Print final order history report -----
    unsigned int produced[OrderTypeN];
    produced[SpotLimit]  = shared.spot_produced;
    produced[MarketSwap] = shared.market_produced;

    unsigned int* consumed[ExecChainTypeN];
    unsigned int ethConsumed[OrderTypeN];
    unsigned int solConsumed[OrderTypeN];

    ethConsumed[SpotLimit]  = shared.consumed[EthExec][SpotLimit];
    ethConsumed[MarketSwap] = shared.consumed[EthExec][MarketSwap];
    solConsumed[SpotLimit]  = shared.consumed[SolExec][SpotLimit];
    solConsumed[MarketSwap] = shared.consumed[SolExec][MarketSwap];

    consumed[EthExec] = ethConsumed;
    consumed[SolExec] = solConsumed;

    log_order_history(produced, consumed);

    // ----- Destroy shared data and exit -----
    destroySharedData(&shared);

    return 0;
}

