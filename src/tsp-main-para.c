#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <assert.h>
#include <complex.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "tsp-types.h"
#include "tsp-job.h"
#include "tsp-genmap.h"
#include "tsp-print.h"
#include "tsp-tsp.h"


/* macro de mesure de temps, retourne une valeur en nanosecondes */
#define TIME_DIFF(t1, t2) \
    ((t2.tv_sec - t1.tv_sec) * 1000000000ll + (long long int) (t2.tv_nsec - t1.tv_nsec))


/* tableau des distances */
tsp_distance_matrix_t distance ={};

/** Param�tres **/

/* nombre de villes */
int nb_towns=10;
/* graine */
long int myseed= 0;
/* nombre de threads */
int nb_threads=1;

/* variables partag�es */
int sol_len;
long long int cuts = 0;
tsp_path_t sol;
struct tsp_queue q;

pthread_mutex_t mutex[MUT_LENGTH];

/* affichage SVG */
bool affiche_sol= false;

static void generate_tsp_jobs (struct tsp_queue *q, int hops, int len, tsp_path_t path, long long int *cuts, tsp_path_t sol, int *sol_len, int depth)
{
    if (len >= minimum) {
        (*cuts)++ ;
        return;
    }

    if (hops == depth) {
        /* On enregistre du travail � faire plus tard... */
        add_job (q, path, hops, len);
    } else {
        int me = path [hops - 1];        
        for (int i = 0; i < nb_towns; i++) {
            if (!present (i, hops, path)) {
                path[hops] = i;
                int dist = distance[me][i];
                generate_tsp_jobs (q, hops + 1, len + dist, path, cuts, sol, sol_len, depth);
            }
        }
    }
}

static void usage(const char *name) {
    fprintf (stderr, "Usage: %s [-s] <ncities> <seed> <nthreads>\n", name);
    exit (-1);
}

void* doWork (void* arg) {
    tsp_path_t sol_thread;
    memset (sol_thread, -1, MAX_TOWNS * sizeof (int));
    sol_thread[0] = 0;
    long long int cuts_thread = 0;
    int min_thread = INT_MAX;
    while (!empty_queue (&q)) {
        int hops = 0, len = 0;
        get_job (&q, sol_thread, &hops, &len);
        // queue
        
        pthread_mutex_lock(&mutex[MUT_MINIMUM]);
        min_thread = minimum;
        pthread_mutex_unlock(&mutex[MUT_MINIMUM]);

        tsp (hops, len, sol_thread, &cuts_thread, sol, &sol_len, &min_thread);
        // cuts, sol_len, solution
    }
    pthread_mutex_lock(&mutex[MUT_CUTS]);
    cuts += cuts_thread;
    pthread_mutex_unlock(&mutex[MUT_CUTS]);
    return (void*)0;
}

int main (int argc, char **argv)
{
    unsigned long long perf;
    struct timespec t1, t2;
    pthread_t* threads;
    tsp_path_t path;

    /* lire les arguments */
    int opt;
    while ((opt = getopt(argc, argv, "s")) != -1) {
        switch (opt) {
            case 's':
                affiche_sol = true;
                break;
            default:
                usage(argv[0]);
                break;
        }
    }

    if (optind != argc-3)
        usage(argv[0]);

    nb_towns = atoi(argv[optind]);
    myseed = atol(argv[optind+1]);
    nb_threads = atoi(argv[optind+2]);
    threads = (pthread_t*)malloc(sizeof(pthread_t)*nb_threads);
    assert(nb_towns > 0);
    assert(nb_threads > 0);

    minimum = INT_MAX;

    /* generer la carte et la matrice de distance */
    fprintf (stderr, "ncities = %3d\n", nb_towns);
    genmap ();

    init_queue (&q);

    clock_gettime (CLOCK_REALTIME, &t1);

    memset (path, -1, MAX_TOWNS * sizeof (int));
    path[0] = 0;

    /* mettre les travaux dans la file d'attente */
    generate_tsp_jobs (&q, 1, 0, path, &cuts, sol, & sol_len, 3);
    no_more_jobs (&q);

    /* calculer chacun des travaux */
    for (int i=0; i < MUT_LENGTH; i++) {
        pthread_mutex_init(&mutex[i], NULL);
    }

    for (int i=0; i < nb_threads; i++) {
        pthread_create(&threads[i], NULL, doWork, (void*)NULL);
    }

    void* status;
    for (int i=0; i < nb_threads; i++) {
        pthread_join(threads[i], &status);
    }
    for (int i=0; i < MUT_LENGTH; i++) {
        pthread_mutex_destroy(&mutex[i]);
    }

    clock_gettime (CLOCK_REALTIME, &t2);

    if (affiche_sol)
        print_solution_svg (sol, sol_len);

    perf = TIME_DIFF (t1,t2);
    printf("<!-- # = %d seed = %ld len = %d threads = %d time = %lld.%03lld ms ( %lld coupures ) -->\n",
            nb_towns, myseed, sol_len, nb_threads,
            perf/1000000ll, perf%1000000ll, cuts);

    return 0 ;
}
