/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#ifndef SMLT_UMP_QUEUEPAIR_H_
#define SMLT_UMP_QUEUEPAIR_H_ 1

#include <ump/smlt_ump_queue.h>

struct smlt_qp;
struct smlt_message;

/**
 * represents an bidirectional UMP queue
 */
struct smlt_ump_queuepair
{
    struct smlt_ump_queue tx;         ///< transmit queue
    struct smlt_ump_queue rx;         ///< receive queue
    struct smlt_ump_queuepair *other; ///< pointer to the other queue pair
    smlt_ump_idx_t seq_id;            ///< last sequence number of the message
    smlt_ump_idx_t last_ack;          ///< last ACKed sequence number
};

/**
 * @brief destorys a ump queuepair
 *
 * @param num_slots   the UMP queuepair
 *
 * @returns SMLT_SUCCESS or error value
 */
errval_t smlt_ump_queuepair_init(smlt_ump_idx_t num_slots, uint8_t node_src,
                                  uint8_t node_dst,
                                  struct smlt_ump_queuepair *src,
                                  struct smlt_ump_queuepair *dst);

/**
 * @brief destorys a ump queuepair
 *
 * @param qp    the UMP queuepair
 *
 * @returns SMLT_SUCCESS or error value
 */
errval_t smlt_ump_queuepair_destroy(struct smlt_ump_queuepair *qp);


/*
 * ===========================================================================
 * Send Functions
 * ===========================================================================
 */

/**
 * @brief checks whether on the queuepair can be sent something
 *
 * @param qp    the UMP queue pair
 *
 * @returns TRUE if message can be sent, FALSE if the queue is full
 */
static inline volatile bool smlt_ump_queuepair_can_send_raw(struct smlt_ump_queuepair *qp)
{
    if ((smlt_ump_idx_t)(qp->seq_id - qp->last_ack) <= qp->tx.num_msg) {
        return true;
    }

    qp->last_ack = smlt_ump_queue_last_ack(&qp->tx);

    return (smlt_ump_idx_t)(qp->seq_id - qp->last_ack) <= qp->tx.num_msg;
}

/**
 * @brief prepares the UMP queuepair to send a message
 *
 * @param qp    the UMP queuepair to prepare
 * @param msg   returns the message pointer
 *
 * @returns SMLT_SUCCESS if the queuepair is prepared, error value otherwise
 */
static inline errval_t smlt_ump_queuepair_prepare_send(struct smlt_ump_queuepair *qp,
                                                       struct smlt_ump_message **msg)
{
    if (!smlt_ump_queuepair_can_send_raw(qp)) {
        return SMLT_ERR_CHAN_WOULD_BLOCK;
    }

    *msg = (struct smlt_ump_message *)smlt_ump_queue_get_next(&qp->tx);

    return SMLT_SUCCESS;
}

static inline errval_t smlt_ump_queuepair_notify_raw(struct smlt_ump_queuepair *qp)
{
    union smlt_ump_ctrl ctrl;

    SMLT_ASSERT(smlt_ump_queuepair_can_send_raw(qp));

    ctrl.c.last_ack = (uintptr_t)qp->seq_id;
    qp->seq_id++;

    smlt_ump_queue_notify(&qp->tx, ctrl);

    return SMLT_SUCCESS;
}

static inline errval_t smlt_ump_queuepair_send_raw(struct smlt_ump_queuepair *qp,
                                                   struct smlt_ump_message *msg)
{
    union smlt_ump_ctrl ctrl;

    SMLT_ASSERT(smlt_ump_queuepair_can_send_raw(qp));

    ctrl.c.last_ack = qp->seq_id;

    qp->seq_id++;
    smlt_ump_queue_send(&qp->tx, msg, ctrl);

    return SMLT_SUCCESS;
}

/* send function pointer */

/**
 * @brief sends a message on the queuepair
 *
 * @param qp     The smelt queuepair to send on
 * @param msg    the Smelt message to send
 *
 * @returns SMELT_SUCCESS of the messessage could be sent.
 */
errval_t smlt_ump_queuepair_try_send(struct smlt_qp *qp,
                                     struct smlt_msg *msg);

/**
 * @brief sends a message on the queuepair
 *
 * @param qp     The smelt queuepair to send on
 * @param msg    the Smelt message to send
 *
 * @returns SMELT_SUCCESS of the messessage could be sent.
 */
static inline errval_t smlt_ump_queuepair_send(struct smlt_qp *qp,
                                               struct smlt_msg *msg)
{
    errval_t err;
    do {
        err = smlt_ump_queuepair_try_send(qp, msg);
    } while(err == SMLT_ERR_QUEUE_FULL);

    return err;
}

/**
* @brief sends a notification on the queuepair
*
* @param qp     The smelt queuepair to send on
*
* @returns SMELT_SUCCESS of the messessage could be sent.
*/
errval_t smlt_ump_queuepair_notify(struct smlt_qp *qp);

/**
* @brief checks if a message can be setn
*
* @param qp     The smelt queuepair to send on
*
* @returns TRUE if a message can be sent, FALSE otherwise
*/
bool smlt_ump_queuepair_can_send(struct smlt_qp *qp);


/*
 * ===========================================================================
 * Receive Functions
 * ===========================================================================
 */


 /**
  * @brief checks (polls) whether a new message can be sent on the queuepair
  *
  * @param qp    the UMP queuepair to check
  *
  * @returns TRUE if there is something pending on the queue, FALSE otherwise
  */
 static inline volatile bool smlt_ump_queuepair_can_recv_raw(struct smlt_ump_queuepair *qp)
 {
     return smlt_ump_queue_can_recv(&qp->rx);
 }

 /**
  * @brief Retrieve an incoming message, if present from the queuepair
  *
  * @param qp    the UMP queuepair
  * @param msg   optional pointer to return a pointer to the raw message
  *
  * \returns Pointer to message on success, NULL if no message is present.
  *
  * Upon successful return, the next call to ump_chan_tx_submit() or
  * ump_chan_send_ack() will send an acknowledgement covering this message. So,
  * the caller must ensure that its payload is consumed or copied to stable
  * storage beforehand.
  *
  * The returned message pointer is the raw message in the channel. The caller
  * must copy its contents.
  */
 static inline errval_t smlt_ump_queuepair_recv_raw(struct smlt_ump_queuepair *qp,
                                                    struct smlt_ump_message **msg)
 {
     if (!smlt_ump_queuepair_can_recv_raw(qp)) {
         return SMLT_ERR_QUEUE_EMPTY;
     }

     return smlt_ump_queue_recv_raw(&qp->rx, msg);
 }

/* recv function pointer */

/**
 * @brief receives a message on the queuepair
 *
 * @param qp     The smelt queuepair to send on
 * @param msg    the Smelt message to receive in
 *
 * @returns SMELT_SUCCESS of the messessage could be received.
 */
errval_t smlt_ump_queuepair_try_recv(struct smlt_qp *qp,
                                     struct smlt_msg *msg);

/**
 * @brief receives a message on the queuepair
 *
 * @param qp     The smelt queuepair to send on
 * @param msg    the Smelt message to receive in
 *
 * @returns SMELT_SUCCESS of the messessage could be received.
 */
static inline errval_t smlt_ump_queuepair_recv(struct smlt_qp *qp,
                                               struct smlt_msg *msg)
{
    errval_t err;
    do {
        err =  smlt_ump_queuepair_try_recv(qp, msg);
    } while(err == SMLT_ERR_QUEUE_EMPTY);

    return err;
}

/**
* @brief receives a notification on the queuepair
*
* @param qp     The smelt queuepair to send on
*
* @returns SMELT_SUCCESS of the messessage could be received.
*/
errval_t smlt_ump_queuepair_recv_notify(struct smlt_qp *qp);

/**
* @brief checks if a message can be received (polling)
*
* @param qp     The smelt queuepair to send on
*
* @returns TRUE if a message can be received, FALSE otherwise
*/
bool smlt_ump_queuepair_can_recv(struct smlt_qp *qp);


#endif
