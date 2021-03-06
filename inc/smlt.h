/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#ifndef SMLT_SMLT_H_
#define SMLT_SMLT_H_ 1

#include <stddef.h>

#include <smlt_config.h>
#include <smlt_platform.h>
#include <smlt_error.h>
#include <smlt_message.h>


//#define SMLT_ASSERT(x) assert(x)
#define SMLT_ASSERT(x)

#define SMLT_ASSERT_CONCAT_(a, b) a##b
#define SMLT_ASSERT_CONCAT(a, b) SMLT_ASSERT_CONCAT_(a, b)

#define SMLT_STATIC_ASSERT(e) \
    enum { SMLT_ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }


/*
 * ensure the SMLT_VERSION is set all the time. 
 */
#ifndef SMLT_VERSION
#define SMLT_VERSION "unknown"
#endif

/*
 * ===========================================================================
 * type declarations
 * ===========================================================================
 */


///< the smelt node id        XXX or is this the endpoint id ?
typedef uint32_t smlt_nid_t;

/**
 * represents a handle to a smelt instance.
 */
struct smlt_instance;

/*
 * ===========================================================================
 * function declarations
 * ===========================================================================
 */

#define SMLT_USE_ALL_CORES ((uint32_t)-1)

/**
 * @brief initializes the Smelt library
 *
 * @param num_proc  the number of processors
 * @param eagerly   create an all-to-all connection mesh
 *
 * @returns SMLT_SUCCESS on success
 *
 * This has to be executed once per address space. If threads are used
 * for parallelism, call this only once. With processes, it has to be
 * executed on each process.
 */
errval_t smlt_init(uint32_t num_proc, bool eagerly);

/**
 * @brief obtains the node based on the id
 *
 * @param id    node id
 *
 * @return pointer ot the Smelt node
 */
struct smlt_node *smlt_get_node_by_id(smlt_nid_t id);

/*
 * ===========================================================================
 * sending functions
 * ===========================================================================
 */


/**
 * @brief sends a message on the to the node
 *
 * @param ep    the Smelt node to call the operation on
 * @param msg   Smelt message argument
 *
 * @returns error value
 *
 * This function is BLOCKING if the node cannot take new messages
 */
errval_t smlt_send(smlt_nid_t nid, struct smlt_msg *msg);

/**
 * @brief sends a notification (zero payload message)
 *
 * @param node    the Smelt node to call the operation on
 * @param msg   Smelt message argument
 *
 * @returns error value
 */
errval_t smlt_notify(smlt_nid_t nid);

/**
 * @brief checks if the a message can be sent on the node
 *
 * @param node    the Smelt node to call the check function on
 *
 * @returns TRUE if the operation can be executed
 *          FALSE otherwise
 */
bool smlt_can_send(smlt_nid_t nid);

/* TODO: include also non blocking variants ? */

/*
 * ===========================================================================
 * receiving functions
 * ===========================================================================
 */


/**
 * @brief receives a message or a notification from the node
 *
 * @param node    the Smelt node to call the operation on
 * @param msg   Smelt message argument
 *
 * @returns error value
 *
 * this function is BLOCKING if there is no message on the node
 */
errval_t smlt_recv(smlt_nid_t nid, struct smlt_msg *msg);

/**
 * @brief checks if there is a message to be received
 *
 * @param node    the Smelt node to call the check function on
 *
 * @returns TRUE if the operation can be executed
 *          FALSE otherwise
 *
 * this invokes either the can_send or can_receive function
 */
bool smlt_can_recv(smlt_nid_t nid);

/**
 * @brief returns the number of processes (or threads) running
 *
 * @returns number of processes/threads participating
 *
 * this invokes either the can_send or can_receive function
 */
uint32_t smlt_get_num_proc(void);

#endif /* SMLT_SMLT_H_ */
