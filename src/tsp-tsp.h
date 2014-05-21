#ifndef TSP_TSP_H
#define TSP_TSP_H

#include <pthread.h>

#define MUT_SOL         0
#define MUT_SOL_LEN     1
#define MUT_CUTS        2
#define MUT_QUEUE       3
#define MUT_MINIMUM     4

#define MUT_LENGTH      5

/* dernier minimum trouvé */
extern int minimum;

extern pthread_mutex_t mutex[MUT_LENGTH];

int present (int city, int hops, tsp_path_t path);
void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len);

#endif
