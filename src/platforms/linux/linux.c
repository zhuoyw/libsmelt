/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <smlt.h>
#include <smlt_node.h>
#include "../../internal.h"

#include <sched.h>

#include <stdarg.h>
#include <numa.h>


/**
 * @brief initializes the platform specific backend
 *
 * @param num_proc  the requested number of processors
 *
 * @returns the number of available processors
 */
uint32_t smlt_platform_init(uint32_t num_proc)
{
    SMLT_DEBUG(SMLT_DBG__INIT, "platform specific initialization (Linux)\n");

    if (numa_available() != 0) {
        SMLT_ERROR("NUMA is not available!\n");
        return 0;
    }
#ifdef SMLT_UMP_ENABLE_SLEEP
    SMLT_WARNING("UMP sleep is enabled - this is buggy\n");
#else
    SMLT_NOTICE("UMP sleep disabled\n");
#endif

    if (num_proc > numa_num_possible_cpus()) {
        num_proc = numa_num_possible_cpus();
    }

    return num_proc;
}

/*
 * ===========================================================================
 * Barrier implementation for initialization purposes
 * ===========================================================================
 */

/**
 * @brief initializes a platform independent barrier for init synchro
 *
 * @param bar     pointer to the barrier memory area
 * @param attr    attributes for the barrier if any
 * @param count   number of threads
 *
 * @returns SMELT_SUCCESS or error value
 */
errval_t smlt_platform_barrier_init(smlt_platform_barrier_t *bar,
                                    smlt_platform_barrierattr_t *attr,
                                    uint32_t count)
{
    return pthread_barrier_init(bar, attr, count);
}

/**
 * @brief destroys a created barrier
 *
 * @param bar     pointer to the barrier

 * @returns SMELT_SUCCESS or error value
 */
errval_t smlt_platform_barrier_destroy(smlt_platform_barrier_t *barrier)
{
    return pthread_barrier_destroy(barrier);
}

/**
 * @brief waits on the barrier untill all threads have entered the barrier
 *
 * @param bar     pointer to the barrier
 *
 * @returns SMELT_SUCCESS or error value
 */
errval_t smlt_platform_barrier_wait(smlt_platform_barrier_t *barrier)
{
    return pthread_barrier_wait(barrier);
}


/*
 * ===========================================================================
 * Thread Control
 * ===========================================================================
 */

/**
 * @brief pins the calling thread to the given core
 *
 * @param core  ID of the core to pint the thread to
 *
 * @return SMLT_SUCCESS or error value
 */
errval_t smlt_platform_pin_thread(coreid_t core)
{
    cpu_set_t cpu_mask;
    int err;

    CPU_ZERO(&cpu_mask);
    CPU_SET(core, &cpu_mask);

    err = sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask);
    if (err) {
        SMLT_ERROR("sched_setaffinity");
        exit(1);
    }

    return SMLT_SUCCESS;
}

/**
 * @brief executed when the Smelt thread is initialized
 *
 * @return SMLT_SUCCESS on ok
 */
errval_t smlt_platform_thread_start_hook(void)
{
#ifdef UMP_DBG_COUNT
#ifdef FFQ
#else
    ump_start();
#endif
#endif
    return SMLT_SUCCESS;
}

/**
 * @brief executed when the Smelt thread terminates
 *
 * @return SMLT_SUCCESS on ok
 */
errval_t smlt_platform_thread_end_hook(void)
{
#ifdef UMP_DBG_COUNT
#ifdef FFQ
#else
    ump_end();
#endif
#endif
    return SMLT_SUCCESS;
}

/*
 * ===========================================================================
 * Platform specific Smelt node management
 * ===========================================================================
 */

/**
 * @brief handles the platform specific initialization of the Smelt node
 *
 * @param node  the node to be created
 *
 * @return SMELT_SUCCESS or error value
 */
errval_t smlt_platform_node_create(struct smlt_node *node)
{
    /*
     * Create the platform specific data structures here
     * i.e. bindings / sockets / file descriptors etc.
     */
    return SMLT_SUCCESS;
}

static void *smlt_platform_node_start_wrapper(void *arg)
{
    SMLT_DEBUG(SMLT_DBG__PLATFORM, "thread started execution\n");

    struct smlt_node *node = arg;

    /* initializing smelt node */
    smlt_node_exec_start(node);

    void *result = smlt_node_run(node);

    /* ending Smelt node */
    smlt_node_exec_end(node);

    SMLT_DEBUG(SMLT_DBG__PLATFORM, "thread ended execution\n");
    return result;
}

/**
 * @brief Starts the platform specific execution of the Smelt node
 *
 * @param node  the Smelt node
 *
 * @return SMELT_SUCCESS or error value
 */
errval_t smlt_platform_node_start(struct smlt_node *node)
{
    SMLT_DEBUG(SMLT_DBG__PLATFORM, "platform: starting node\n")
    int err = pthread_create(&node->handle, NULL, smlt_platform_node_start_wrapper,
                             node);
    if (err) {
        return -1;
    }

    return SMLT_SUCCESS;
}

/**
 * @brief waits for the Smelt node to be done with execution
 *
 * @param node  the Smelt node
 *
 * @return SMELT_SUCCESS or error value
 */
errval_t smlt_platform_node_join(struct smlt_node *node)
{
    return SMLT_SUCCESS;
}

/**
 * @brief cancles the execution of the Smelt node
 *
 * @param node  The Smelt node
 *
 * @return SMELT_SUCCESS or error value
 */
errval_t smlt_platform_node_cancel(struct smlt_node *node)
{

    return SMLT_SUCCESS;
}


/*
 * ===========================================================================
 * Memory allocation abstraction
 * ===========================================================================
 */


/**
 * @brief allocates a buffer reachable by all nodes on this machine
 *
 * @param bytes         number of bytes to allocate
 * @param align         align the buffer to a multiple bytes
 * @param do_clear      if TRUE clear the buffer (zero it)
 *
 * @returns pointer to newly allocated buffer
 *
 * When processes are used this buffer will be mapped to all processes.
 * The memory is allocated from any numa node
 */
void *smlt_platform_alloc(uintptr_t bytes, uintptr_t align, bool do_clear)
{
    SMLT_WARNING("%s not fully implemented! (alignment)\n", __FUNCTION__);
    if (do_clear) {
        return calloc(1, bytes);
    }
    return malloc(bytes);
}

/**
 * @brief allocates a buffer reachable by all nodes on this machine on a NUMA node
 *
 * @param bytes     number of bytes to allocate
 * @param align     align the buffer to a multiple bytes
 * @param node      which numa node to allocate the buffer
 * @param do_clear  if TRUE clear the buffer (zero it)
 *
 * @returns pointer to newly allocated buffer
 *
 * When processes are used this buffer will be mapped to all processes.
 * The memory is allocated form the specified NUMA node
 */
void *smlt_platform_alloc_on_node(uint64_t bytes, uintptr_t align, uint8_t node,
                                  bool do_clear)
{
    SMLT_WARNING("%s not fully implemented! (NUMA node)\n", __FUNCTION__);
    return smlt_platform_alloc(bytes, align, do_clear);
}

/**
 * @brief frees the buffer
 *
 * @param buf   the buffer to be freed
 */
void smlt_platform_free(void *buf)
{
    free(buf);
}

#if 0


/**
 * \brief Get core ID of the current thread/process
 *
 * Assumption: On Linux, the core_id equals the thread_id
 */
coreid_t get_core_id(void)
{
    return get_thread_id();
}

int _tree_prepare(void)
{
    return 0;
}

void _tree_export(const char *qrm_my_name)
{
    // Linux does not need any export
}

void _tree_register_receive_handler(struct sync_binding *)
{
}

/* --------------------------------------------------
 * Barrelfish compatibility
 * --------------------------------------------------
 */
coreid_t disp_get_core_id(void)
{
    return get_thread_id();
}

void bench_init(void)
{
}

cycles_t bench_tscoverhead(void)
{
    return 0;
}

/* --------------------------------------------------
 * Tree operations
 * --------------------------------------------------
 */

int
numa_cpu_to_node(int cpu)
{
    int ret    = -1;
    int node_max = numa_max_node();
    struct bitmask *cpus = numa_allocate_cpumask();

    for (int node=0; node <= node_max; node++) {
        numa_bitmask_clearall(cpus);
        if (numa_node_to_cpus(node, cpus) < 0) {
            perror("numa_node_to_cpus");
            fprintf(stderr, "numa_node_to_cpus() failed for node %d\n", node);
            if (errno== ERANGE) {
                fprintf(stderr, "Given range is too small\n");
            }
            abort();
        }

        if (numa_bitmask_isbitset(cpus, cpu)) {
            ret = node;
        }
    }

    numa_free_cpumask(cpus);

    if (ret == -1) {
        fprintf(stderr, "%s failed to find node for cpu %d\n",
                __FUNCTION__, cpu);
        abort();
    }

    return ret;
}


#ifdef FFQ
// --------------------------------------------------

#define FF_CONF_INIT(src_, dst_)                        \
    {                                                   \
        .src = {                                        \
            .core_id = src_,                            \
            .delay = 0,                                 \
        },                                              \
        .dst = {                                        \
            .core_id = dst_,                            \
            .delay = 0                                  \
        },                                              \
        .next = NULL,                                   \
        .qsize = UMP_QUEUE_SIZE,                        \
        .shm_numa_node = -1                             \
    }

/**
 * \brief Setup a pair of UMP channels.
 *
 * This is borrowed from UMPQ's pt_bench_pairs_ump program.
 */
void _setup_chanels(int src, int dst)
{
    debug_printfff(DBG__INIT, "FF: setting up channel between %d and %d\n",
                   src, dst);

    struct ffq_pair_conf ffpc = FF_CONF_INIT(src, dst);
    struct ffq_pair_state *ffps = ffq_pair_state_create(&ffpc);

    add_binding(src, dst, ffps);

    struct ffq_pair_conf ffpc_r = FF_CONF_INIT(dst, src);
    struct ffq_pair_state *ffps_r = ffq_pair_state_create(&ffpc_r);

    add_binding(dst, src, ffps_r);
}

void mp_send_raw7(mp_binding *b,
                  uintptr_t val1,
                  uintptr_t val2,
                  uintptr_t val3,
                  uintptr_t val4,
                  uintptr_t val5,
                  uintptr_t val6,
                  uintptr_t val7)
{
    ffq_enqueue_full(&b->src, val1, val2, val3,
                val4, val5, val6, val7);
}

void mp_receive_raw7(mp_binding *b, uintptr_t* buf)
{
    ffq_dequeue_full(&b->dst, buf);
}

bool mp_can_receive_raw(mp_binding *b)
{
    return ffq_can_dequeue(&b->dst);
}

#else // UMP
// --------------------------------------------------

#define UMP_CONF_INIT(src_, dst_, shm_size_)      \
{                                                 \
    .src = {                                      \
         .core_id = src_,                         \
         .shm_size = shm_size_,                   \
         .shm_numa_node = numa_cpu_to_node(src_)  \
    },                                            \
    .dst = {                                      \
         .core_id = dst_,                         \
         .shm_size = shm_size_,                   \
         .shm_numa_node = numa_cpu_to_node(src_)  \
    },                                            \
    .shared_numa_node = -1,                       \
    .nonblock = 1                                 \
}

/**
 * \brief Setup a pair of UMP channels.
 *
 * This is borrowed from UMPQ's pt_bench_pairs_ump program.
 */
void _setup_chanels(int src, int dst)
{
    debug_printfff(DBG__INIT, "Establishing connection between %d and %d\n",
                   src, dst);

    const int shm_size = (64*UMP_QUEUE_SIZE);

    struct ump_pair_conf fwr_conf = UMP_CONF_INIT(src, dst, shm_size);
    struct ump_pair_conf rev_conf = UMP_CONF_INIT(dst, src, shm_size);

    add_binding(src, dst, ump_pair_state_create(&fwr_conf));
    add_binding(dst, src, ump_pair_state_create(&rev_conf));
}

void mp_send_raw(mp_binding *b, uintptr_t val)
{
    struct ump_pair_state *ups = (struct ump_pair_state*) b;
    struct ump_queue *q = &ups->src.queue;

    ump_enqueue_word(q, val);
}

void mp_send_raw7(mp_binding *b,
                  uintptr_t val1,
                  uintptr_t val2,
                  uintptr_t val3,
                  uintptr_t val4,
                  uintptr_t val5,
                  uintptr_t val6,
                  uintptr_t val7)
{
    struct ump_pair_state *ups = (struct ump_pair_state*) b;
    struct ump_queue *q = &ups->src.queue;

    ump_enqueue(q, val1, val2, val3, val4,
                val5, val6, val7);
}


uintptr_t mp_receive_raw(mp_binding *b)
{
    struct ump_pair_state *ups = (struct ump_pair_state*) b;
    struct ump_queue *q = &ups->dst.queue;

    uintptr_t r;
    ump_dequeue_word(q, &r);

    return r;
}


void mp_receive_raw7(mp_binding *b, uintptr_t* buf)
{
    struct ump_pair_state *ups = (struct ump_pair_state*) b;
    struct ump_queue *q = &ups->dst.queue;

    ump_dequeue(q, buf);
}

bool mp_can_receive_raw(mp_binding *b)
{
    struct ump_pair_state *ups = (struct ump_pair_state*) b;
    struct ump_queue *q = &ups->dst.queue;
    return ump_can_dequeue(q);
}

#endif

/**
 * \brief Establish connections as given by the model.
 */
void tree_connect(const char *qrm_my_name)
{
    uint64_t nproc = topo_num_cores();

    for (unsigned int i=0; i<nproc; i++) {

        for (unsigned int j=0; j<nproc; j++) {

            if (topo_is_parent_real(i, j) ||
                (j==get_sequentializer() && i!=j && topo_does_mp(i))) {
                debug_printfff(DBG__SWITCH_TOPO, "setup: %d %d\n", i, j);
                _setup_chanels(i, j);
            }
        }
    }
}

void debug_printf(const char *fmt, ...)
{
    va_list argptr;
    char str[1024];
    size_t len;

    len = snprintf(str, sizeof(str), "\033[34m%d\033[0m: ",
                   get_thread_id());
    if (len < sizeof(str)) {
        va_start(argptr, fmt);
        vsnprintf(str + len, sizeof(str) - len, fmt, argptr);
        va_end(argptr);
    }
    printf(str, sizeof(str));
}





#endif
