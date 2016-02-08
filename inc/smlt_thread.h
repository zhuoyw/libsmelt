/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#ifndef SMLT_THREAD_H_
#define SMLT_THREAD_H_ 1

/*
 * ===========================================================================
 * type declarations
 * ===========================================================================
 */

///< the smelt thread id        XXX or is this the endpoint id ?
typedef uint32_t smlt_tid_t;

/*
 * ===========================================================================
 * thread management functions
 * ===========================================================================
 */ 
errval_t smlt_thread_create();

errval_t smlt_thread_join();

errval_t smlt_thread_cancel();

/*
 * ===========================================================================
 * thread state functions
 * ===========================================================================
 */ 

smlt_tid_t smlt_thread_get_thread_id(void);

/// xxx we need to have a way to get the <machine:core>
coreid_t smlt_thread_get_core_id();

bool smlt_thread_is_coordinator();

uint32_t smlt_thread_get_num_thread();



mp_binding *mp_get_parent(coreid_t, int*);
mp_binding **mp_get_children(coreid_t, int*, int**);
void mp_connect(coreid_t src, coreid_t dst);



#endif /* SMLT_THREAD_H_ */