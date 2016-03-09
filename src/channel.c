/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <smlt_platform.h>
#include <smlt_error.h>
#include <smlt_queuepair.h>
#include <smlt_channel.h>

/*
 * ===========================================================================
 * Smelt channel creation and destruction
 * ===========================================================================
 */

 /**
  * @brief creates the queue pair
  *
  * @param type     type of the channel (o2o o2m m2o m2m)
  * @param chan     return pointer to the channel
  * @param src      src core id // TODO use nid ? 
  * @param dst      array of core ids to desinations
  * @param count    length of array dst;   
  *
  * @returns SMLT_SUCCESS or failure 
  */
errval_t smlt_channel_create(struct smlt_channel **chan,
                             uint32_t* src,
                             uint32_t* dst,
                             uint16_t count_src,
                             uint16_t count_dst)
{
    uint32_t num_chan = (count_src > count_dst) ? count_src : count_dst;

    assert(*chan);

    ((*chan)->recv) = (struct smlt_qp*) smlt_platform_alloc(
                        sizeof(struct smlt_qp)*num_chan,
                        SMLT_DEFAULT_ALIGNMENT, true);
    ((*chan)->send) = (struct smlt_qp*) smlt_platform_alloc(
                        sizeof(struct smlt_qp)*num_chan,
                        SMLT_DEFAULT_ALIGNMENT, true);
    if (!chan) {
        return SMLT_ERR_MALLOC_FAIL;
    }
    
    if ((count_src > 1) && (count_dst > 1)) {
        assert(!"M:N Channel NYI");
    }

    // setn
    (*chan)->m = count_dst;
    (*chan)->n = count_src;

    errval_t err;
    for (int i= 0; i < num_chan; i++) {
        if (count_src == 1) {
            // 1_n
            struct smlt_qp* send = &((*chan)->send[i]);
            struct smlt_qp* recv = &((*chan)->recv[i]);
            err = smlt_queuepair_create(SMLT_QP_TYPE_SHM,
                                    &send, &recv, src[0], dst[i]);
            if (smlt_err_is_fail(err)) {
                return smlt_err_push(err, SMLT_ERR_CHAN_CREATE);
            }
        } else {
            // 1:n
            struct smlt_qp* send = &((*chan)->send[i]);
            struct smlt_qp* recv = &((*chan)->recv[i]);
            err = smlt_queuepair_create(SMLT_QP_TYPE_UMP,
                                    &send, &recv, src[i], dst[0]);
            if (smlt_err_is_fail(err)) {
                return smlt_err_push(err, SMLT_ERR_CHAN_CREATE);
            }
        }
    }
    return SMLT_SUCCESS;
}

 /**
  * @brief destroys the channel
  *
  * @param
  *
  * @returns 0
  */
errval_t smlt_channel_destroy(struct smlt_channel *chan)
{
    uint32_t num_chan = (chan->m > chan->n) ? chan->m : chan->n;

    errval_t err;
    for (int i = 0; i < num_chan; i++) {
            err = smlt_queuepair_destroy(&chan->send[i]);
            if (smlt_err_is_fail(err)) {
                return smlt_err_push(err, SMLT_ERR_CHAN_DESTROY);
            }
            smlt_platform_free(chan->send);
            smlt_platform_free(chan->recv);
    }
    return SMLT_SUCCESS;
}