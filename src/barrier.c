#include "sync.h"
#include "shm.h"
#include "barrier.h"


/**
 * @brief destroys a smlt barrier
 *
 * @param bar the Smelt barrier to be initialized
 *
 * @returns TODO:errval
 */
errval_t smlt_barrier_init(struct smlt_barrier *bar)
{


}

/**
 * @brief destroys a smlt barrier
 *
 * @param bar the Smelt barrier to destroy
 *
 * @returns TODO:errval
 */
errval_t smlt_barrier_destroy(struct smlt_barrier *bar)
{


}

/**
 * @brief waits on the supplied barrier
 *
 * @param bar the Smelt barrier to wait on
 *
 * @returns TODO:errval
 */
errval_t smlt_barrier_wait(struct smlt_barrier *bar)
{
    smlt_reduce_notify();
    smlt_broadcast_notify();
}





void shl_barrier_shm(int b_count);

/**
 * Shoal's barrier implementation.
 *
 * This implementation currently has many limitations:
 *
 * - only one instance of a barrier is supported. The reason for this
     is that we want shared state for counters to be stored on the
     master share. The latter is shared even if processes are used
     rather than threads. A clean solution would be to allocate a new
     shared thread for each barrier and use that instead.
 *
 */

static int idx_max = 0;
int shl_barrier_destroy(shl_barrier_t *barrier)
{
    idx_max--;
    return 0;
}

static unsigned barrier_num = 0;
int shl_barrier_init(shl_barrier_t *barrier,
                     const void *attr, unsigned count)
{
    //    assert (idx_max==0); // only one barrier at a time.

    printf("shl_barrier_init count %d\n", count);

    assert (idx_max==0 || barrier_num==count);

    if (idx_max>0) {
        printf("WARNING: Mapping multiple barriers to the same implementation\n");
    }

    barrier_num = count;
    barrier->num = count;
    barrier->idx = idx_max++;

    return 0;
}

int shl_barrier_wait(shl_barrier_t *barrier)
{
    //    assert (barrier->idx==0);
    shl_barrier_shm(barrier->num);

    return 0;
}

int shl_hybrid_barrier(void* bla)
{
    // Reduce
    sync_reduce(1);

    // Broadcast
    ab_forward(1, get_sequentializer());

    return 0;
}

int shl_hybrid_barrier0(void* bla)
{
    // Reduce
    sync_reduce0(1);

    // Broadcast
    ab_forward0(1, get_sequentializer());

    return 0;
}


static __thread int _shl_round = 0;
void shl_barrier_shm(int b_count)
{
#if defined(QRM_DBG_ENABLED)
    assert(_shl_round<QRM_ROUND_MAX);
#endif

#if 0
    assert (!"Not tested with overflow");

    // //////////////////////////////////////////////////
    // SPINLOCK (disabled)
    // //////////////////////////////////////////////////
    volatile uint8_t *ptr = &(get_master_share()->data.rounds[round]);

    // Increment counter
    acquire_spinlock(&get_master_share()->data.lock);
    *ptr += 1;
    QDBG("SPINLOCK: core %d waiting in round %d ptr %d\n",
           get_thread_id(), round, *ptr);
    release_spinlock(&get_master_share()->data.lock);

    // Wait until counter reached number of cores

    int i = 0;
    while (*ptr<b_count) {

        // Yield every now and then ..
        if (i++ >2000) {

            /* thread_yield(); */
            i = 0;
        }

    }

    QDBG("SPINLOCK: core %d leaving in round %d ptr %d\n",
           get_thread_id(), round, *ptr);

#elif 1
    // //////////////////////////////////////////////////
    // Atomic increment
    // //////////////////////////////////////////////////

#if defined(QRM_DBG_ENABLED)
    if (get_master_share()==NULL) {
        printf("cb: quorum %p %p\n",
               __builtin_return_address(0),
               __builtin_return_address(1));
    }
    assert(get_master_share()!=NULL);
#endif

    volatile uint8_t *ptr = &(get_master_share()->data.rounds[_shl_round]);

#if defined(QRM_DBG_ENABLED)
    uint8_t val =
#endif
    __sync_add_and_fetch(ptr, 1);

    debug_printfff(DBG__QRM_BARRIER,
                   "qrm_barrier: core %d value %d waiting for %d round %d\n",
                   get_thread_id(), val, b_count, _shl_round);

#ifdef SIMULATOR
    uint64_t i = 1;
#endif

    while (*ptr<b_count) {

#ifdef SIMULATOR
        // Sleep every now and then .. we should remove this later on
        // We should use this only when running in the SIMULATOR

        // qemu run's nicely when one physical core is simulated by
        // each hyperthread, unless they are spinning on a memory
        // location.
        if ((++i)%100000 == 0) {
            thread_yield();
        }
#endif

    }

    // Everyone passed the barrier, and is working on the next
    // one. Access to this barriers counter is save now to the
    // coordinator, which can reset it's value to 0.
    if (get_thread_id()==get_sequentializer()) {
        unsigned _tmp = (_shl_round+QRM_ROUND_MAX-1)%QRM_ROUND_MAX;
        get_master_share()->data.rounds[_tmp] = 0;
    }

    debug_printfff(DBG__QRM_BARRIER, "qrm_barrier done\n");

#else
    assert (!"Overflow of round will not work");
    // //////////////////////////////////////////////////
    // Octopus
    // //////////////////////////////////////////////////
    char *dummy;
    char name[100];
    sprintf(name, "qrm_exp_round_%d", round);
    oct_barrier_enter(name, &dummy, b_count);
    oct_barrier_leave(dummy);
#endif
    _shl_round = (_shl_round+1) % QRM_ROUND_MAX;
}


void mp_barrier(cycles_t *measurement)
{
    coreid_t tid = get_core_id();

#ifdef QRM_DBG_ENABLED
    ++_num_barrier;
    uint32_t _num_barrier_recv = _num_barrier;
#endif

    debug_printfff(DBG__REDUCE, "barrier enter #%d\n", _num_barrier);

    // Recution
    // --------------------------------------------------
#ifdef QRM_DBG_ENABLED
    uint32_t _tmp =
#endif
    mp_reduce(_num_barrier);

#ifdef QRM_DBG_ENABLED
    // Sanity check
    if (tid==get_sequentializer()) {
        assert (_tmp == get_num_threads()*_num_barrier);
    }
    if (measurement)
        *measurement = bench_tsc();

#endif

    // Broadcast
    // --------------------------------------------------
    if (tid == get_sequentializer()) {
        mp_send_ab(_num_barrier);

    } else {
#ifdef QRM_DBG_ENABLED
        _num_barrier_recv =
#endif
            mp_receive_forward(0);
    }

#ifdef QRM_DBG_ENABLED
    if (_num_barrier_recv != _num_barrier) {
    debug_printf("ASSERTION fail %d != %d\n", _num_barrier_recv, _num_barrier);
    }
    assert (_num_barrier_recv == _num_barrier);

    // Add a shared memory barrier to absolutely make sure that
    // everybody finished the barrier before leaving - this simplifies
    // debugging, as the programm will get stuck if barriers are
    // broken, rather than some threads (wrongly) continuing and
    // causing problems somewhere else
#if 0 // Enable separately
    debug_printfff(DBG_REDUCE, "finished barrier .. waiting for others\n");
    shl_barrier_shm(get_num_threads());
#endif
#endif

    debug_printfff(DBG__REDUCE, "barrier complete #%d\n", _num_barrier);
}
