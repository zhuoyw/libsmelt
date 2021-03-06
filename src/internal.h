/*
 * Copyright (c) 2015 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#ifndef INTERNAL_DEBUG_H
#define INTERNAL_DEBUG_H 1

#include "smlt_debug.h"

//struct shm_context {//
//
 //
//};

/**
 * \brief Per-thread context of libsync.
 *
 * Is setup in __thread_init()
 */
struct libsync_context {

    // NYI    struct mp_context mp_c;

    struct shm_queue *shm_c;
};

/**
 * File: tree_setup.c
 *
 * This header should never be imported by anything outside of libsync.
 */

void tree_reset(void);
int  tree_init(const char*);
void tree_init_bindings(void);

void setup_tree_from_model(void);

/**
 * List of bindings.
 */
struct binding_lst {
    unsigned int num;
  //  mp_binding **b;
  //  mp_binding **b_reverse;
    int *idx;
};

//binding_lst *_mp_get_parent_raw(coreid_t c);
//binding_lst *_mp_get_children_raw(coreid_t c);

//void _setup_chanels(int src, int dst);

/*
 * =============================================================================
 * Node functions
 * =============================================================================
 */

/*
 * =============================================================================
 * Platform specific functions
 * =============================================================================
 */


/**
 * @brief executed when the Smelt thread is initialized
 *
 * @return SMLT_SUCCESS on ok
 */
errval_t smlt_platform_thread_start_hook(void);

/**
 * @brief executed when the Smelt thread terminates
 *
 * @return SMLT_SUCCESS on ok
 */
errval_t smlt_platform_thread_end_hook(void);

#endif /* INTERNAL_DEBUG_H */
