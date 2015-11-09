/*
 *
 */
#include <stdio.h>
#include "sync.h"
#include "topo.h"
#include "mp.h"
#include <pthread.h>

#include "model_defs.h"

#define NUM_THREADS 4
void* worker1(void* a)
{
    int tid = *((int*) a);
    __thread_init(tid, NUM_THREADS);

    if (tid == SEQUENTIALIZER) {

        for (unsigned int i=0; i<topo_num_cores(); i++) {
            if (topo_is_parent(get_thread_id(), i)) {
                mp_send(i, get_thread_id());
            }
        }
    }

    else {

        for (unsigned int i=0; i<topo_num_cores(); i++) {
            if (i==SEQUENTIALIZER && topo_is_parent(i, get_thread_id())) {
                assert (mp_receive(i)==i);
            }
        }
    }

    printf("Thread %d completed\n", tid);
    
    return NULL;
}

#define NUM_RUNS 1000000 // Tested up to 1.000.000
void* worker2(void* a)
{
    int tid = *((int*) a);
    __thread_init(tid, NUM_THREADS);

    for (int epoch=0; epoch<NUM_RUNS; epoch++) {
    
        if (tid == SEQUENTIALIZER) {
            mp_send_ab(tid);

            // Wait for message from LAST_NODE
            mp_receive(LAST_NODE);
        
        } else {
            mp_receive_forward(tid);

            if (get_thread_id()==LAST_NODE) {

                mp_send(SEQUENTIALIZER, 0);
            }
        }
    }
    printf("Thread %d completed\n", tid);
    
    return NULL;
}

void* worker3(void* a)
{
    int tid = *((int*) a);
    __thread_init(tid, NUM_THREADS);
    
    debug_printf("Reduction complete: %d\n", mp_reduce(tid));

    return NULL;
}

void* worker4(void* a)
{
    int tid = *((int*) a);
    __thread_init(tid, NUM_THREADS);
    
    for (int epoch=0; epoch<NUM_RUNS; epoch++) {
        
        //        debug_printf("before barrier\n");
        mp_barrier();
        //        debug_printf("after barrier\n");
    }

    return NULL;
}

#define NUM_EXP 4

int main(int argc, char **argv)
{
    typedef void* (worker_func_t)(void*);
    worker_func_t* workers[NUM_EXP] = {
        &worker1,
        &worker2,
        &worker3,
        &worker4
    };

    __sync_init(NUM_THREADS);

    pthread_t ptds[NUM_THREADS];
    int tids[NUM_THREADS];

    for (int j=0; j<NUM_EXP; j++) {

        printf("----------------------------------------\n");
        printf("Executing experiment %d\n", (j+1));
        printf("----------------------------------------\n");
        
        // Create
        for (int i=0; i<NUM_THREADS; i++) {
            tids[i] = i;
            pthread_create(ptds+i, NULL, workers[j], (void*) (tids+i));
        }

        // Join
        for (int i=0; i<NUM_THREADS; i++) {
            pthread_join(ptds[i], NULL);
        }
    }
    
}