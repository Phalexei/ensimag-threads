#include <assert.h>
#include <string.h>

#include "tsp-types.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"

/* dernier minimum trouvé */
int minimum;

/* résolution du problème du voyageur de commerce */
int present (int city, int hops, tsp_path_t path)
{
    int bRet = 0;
    for (int i = 0; i < hops; i++) {
        if (path[i] == city) {
            bRet = 1;
            break;
        }
    }

    return bRet ;
}


void tsp (int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len)
{
    // thread safe : cuts,hops, len, path
    // non thread-safe : sol, sol_len, minimum
    

    if (len + cutprefix[(nb_towns-hops)] >= minimum) {
        (*cuts)++ ;
        return;
    }

    if (hops == nb_towns) {
        int me = path [hops - 1];
        int dist = distance[me][0]; // retourner en 0
        if ( len + dist < minimum ) {
            minimum = len + dist;
            
            *sol_len = len + dist;


            memcpy(sol, path, nb_towns*sizeof(int));
            print_solution (path, len+dist);
        }
    } else {
        
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {

                path[hops] = i;
                int dist = distance[me][i];
                tsp (hops + 1, len + dist, path, cuts, sol, sol_len);
            }
        }
    }
}

