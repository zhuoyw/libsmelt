#include "sync.h"
#include "topo.h"
#include "shm.h"
#include "mp.h"

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

uintptr_t ab_forward0(uintptr_t val, coreid_t sender)
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
            mp_send_ab0();
        } else {
            mp_receive_forward0();
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

void ab_forward7(coreid_t sender,
                 uintptr_t v1,
                 uintptr_t v2,
                 uintptr_t v3,
                 uintptr_t v4,
                 uintptr_t v5,
                 uintptr_t v6,
                 uintptr_t v7,
                 uintptr_t* msg_buf)
{
    debug_printfff(DBG__HYBRID_AC, "hybrid_ac entered\n");

    coreid_t my_core_id = get_thread_id();
    bool does_mp = topo_does_mp_receive(my_core_id, false) ||
        (topo_does_mp_send(my_core_id, false));

    // Sender forwards message to the root
    if ((get_thread_id()==sender) &&
         !(sender == get_sequentializer())) {

        mp_send7(get_sequentializer(), v1, v2, v3, v4,
                 v5, v6, v7);
    }

    // Message passing
    // --------------------------------------------------
    if (does_mp) {

        debug_printfff(DBG__HYBRID_AC, "Starting message passing .. "
                       "does_recv=%d does_send=%d - val=%d\n",
                       topo_does_mp_receive(my_core_id, false),
                       topo_does_mp_send(my_core_id, false),
                       v1);

        if (my_core_id == SEQUENTIALIZER) {
            if (!(sender == SEQUENTIALIZER)) {
                mp_receive7(sender, msg_buf);
            }
            mp_send_ab7(msg_buf[0], msg_buf[1], msg_buf[2], msg_buf[3],
                        msg_buf[4], msg_buf[5], msg_buf[6]);
        } else {
            mp_receive_forward7(msg_buf);
        }
    }

    // Shared memory
    // --------------------------------------------------
    if (topo_does_shm_send(my_core_id) || topo_does_shm_receive(my_core_id)) {

        if (topo_does_shm_send(my_core_id)) {

            // send
            // --------------------------------------------------

            debug_printfff(DBG__HYBRID_AC, "Starting SHM .. -- send, \n");

            shm_send(msg_buf[0], msg_buf[1], msg_buf[2], msg_buf[3],
                     msg_buf[4], msg_buf[5], msg_buf[6]);
        }
        if (topo_does_shm_receive(my_core_id)) {

            // receive
            // --------------------------------------------------

            debug_printfff(DBG__HYBRID_AC, "Starting SHM .. -- receive, \n");

            shm_receive(&msg_buf[0], &msg_buf[1], &msg_buf[2], &msg_buf[3], &msg_buf[4],
                        &msg_buf[5], &msg_buf[6]);
        }

        debug_printfff(DBG__HYBRID_AC, "End SHM\n");
    }

}