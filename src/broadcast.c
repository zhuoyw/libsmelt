/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#include <smlt.h>
#include <smlt_broadcast.h>


/**
 * @brief performs a broadcast to the nodes of the subtree routed at the 
 *        calling node
 * 
 * @param msg       input for the reduction
 * @param result    returns the result of the reduction
 * 
 * @returns TODO:errval
 */
errval_t smlt_broadcast_subtree(struct smlt_msg *msg)
{
    errval_t err;

    uint32_t count;
    struct smlt_node **nl = smlt_node_get_children(smlt_node_get_self(),
                                                   &count);

    for (uint32_t i = 0; i < count; ++i) {
        err = smlt_node_send(nl[i], msg);
        // TODO: error handling
    }

    return SMLT_SUCCESS;
}

/**
 * @brief performs a broadcast without any payload to the subtree routed at
 *        the calling node
 *
 * @returns TODO:errval
 */
errval_t smlt_broadcast_notify_subtree(void)
{
    errval_t err;
    uint32_t count;
    struct smlt_node **nl = smlt_node_get_children(&count);

    for (uint32_t i = 0; i < count; ++i) {
        err = smlt_node_notify(nl[i]);
        // TODO: error handling
    }

    return SMLT_SUCCESS;
}

/**
 * @brief performs a broadcast on the current active instance 
 * 
 * @param msg       input for the reduction
 * @param result    returns the result of the reduction
 * 
 * @returns TODO:errval
 */
errval_t smlt_broadcast(struct smlt_msg *msg)
{
    errval_t err;

    if (smlt_node_is_root()) {
        err = smlt_broadcast_subtree(msg);
    } else {
        struct smlt_node *p = smlt_node_get_parent();
        err = smlt_node_recv(p, msg);
        /* TODO: ERROR */
        err = smlt_broadcast_subtree(msg);
        /* TODO: ERROR */
    }

    return SMLT_SUCCESS;
}

/**
 * @brief performs a broadcast without any payload
 *
 * @returns TODO:errval
 */
errval_t smlt_broadcast_notify(void)
{
    errval_t err;

    if (smlt_node_is_root()) {
        err = smlt_broadcast_notify_subtree(msg);
    } else {
        struct smlt_node *p = smlt_node_get_parent();
        err = smlt_node_recv(p, msg);
        /* TODO: ERROR */
        err = smlt_broadcast_notify_subtree(msg);
        /* TODO: ERROR */
    }

    return smlt_broadcast_subtree_notify();
}




/**
 * \brief
 *
 */
uintptr_t ab_forward(uintptr_t val, coreid_t sender)
{
    debug_printfff(DBG__HYBRID_AC, "hybrid_ac entered\n");

    coreid_t my_core_id = get_thread_id();
    bool does_mp = topo_does_mp_receive(my_core_id, false) ||
        (topo_does_mp_send(my_core_id, false));

    // Sender forwards message to the root
    if (get_thread_id()==sender && sender!=get_sequentializer()) {

        mp_send(get_sequentializer(), val);
    }

    // Message passing
    // --------------------------------------------------
    if (does_mp) {

        debug_printfff(DBG__HYBRID_AC, "Starting message passing .. "
                       "does_recv=%d does_send=%d - val=%d\n",
                       topo_does_mp_receive(my_core_id, false),
                       topo_does_mp_send(my_core_id, false),
                       val);

        if (my_core_id == SEQUENTIALIZER) {
            if (sender!=get_sequentializer()) {
                val = mp_receive(sender);
            }
            mp_send_ab(val);
        } else {
            val = mp_receive_forward(0);
        }
    }

    // Shared memory
    // --------------------------------------------------
    if (topo_does_shm_send(my_core_id) || topo_does_shm_receive(my_core_id)) {

        if (topo_does_shm_send(my_core_id)) {

            // send
            // --------------------------------------------------

            debug_printfff(DBG__HYBRID_AC, "Starting SHM .. -- send, \n");

            shm_send(val, 0, 0, 0, 0, 0, 0);
        }
        if (topo_does_shm_receive(my_core_id)) {

            // receive
            // --------------------------------------------------

            debug_printfff(DBG__HYBRID_AC, "Starting SHM .. -- receive, \n");

            uintptr_t val1, val2, val3, val4, val5, val6, val7;
            shm_receive(&val1, &val2, &val3, &val4, &val5, &val6, &val7);
        }

        debug_printfff(DBG__HYBRID_AC, "End SHM\n");
    }

    return val;

}