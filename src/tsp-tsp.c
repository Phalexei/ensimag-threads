#include <assert.h>
#include <string.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

/* dernier minimum trouv� */
int minimum;

/* r�solution du probl�me du voyageur de commerce */
int present (int city, int hops, tsp_path_t path)
{
    for (int i = 0; i < hops; i++) {
        if (path[i] == city) {
            return 1;
        }
    }
    return 0;
}


void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int* min)
{
    // thread safe : cuts,hops, len, path, min
    // non thread-safe : sol, sol_len, minimum
    

    if (len + cutprefix[(nb_towns-hops)] >= *min) {
        (*cuts)++ ;
        return;
    }

    if (hops == nb_towns) {
        int me = path [hops - 1];
        int dist = distance[me][0]; // retourner en 0
        if ( len + dist < *min ) {
            pthread_mutex_lock(&mutex[MUT_MINIMUM]);
            if (len + dist < minimum) {
                minimum = len + dist;
           
                *sol_len = len + dist;

                memcpy(sol, path, nb_towns*sizeof(int));
                print_solution (path, len+dist);
            }
            *min = minimum;
            pthread_mutex_unlock(&mutex[MUT_MINIMUM]);
        }
    } else {
        
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {

                path[hops] = i;
                int dist = distance[me][i];
                tsp (hops + 1, len + dist, path, cuts, sol, sol_len, min);
            }
        }
    }
}

