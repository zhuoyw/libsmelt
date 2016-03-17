/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#include <smlt.h>
#include <smlt_topology.h>
#include <smlt_generator.h>
#include "debug.h"
#include <stdio.h>

/**
 * this is a node in the current topology
 */
struct smlt_topology_node
{
    struct smlt_topology *topology;         ///< backpointer to the topology
    struct smlt_topology_node *parent;      ///< pointer to the parent node
    struct smlt_topology_node **children;   ///< array of children
    uint32_t chanel_type;                   ///< queuepairs
    smlt_nid_t node_id;                     ///< qp[0]->parent qp[1..n] child
    uint32_t array_index;                   ///< Invalid if root
    uint32_t num_children;                  ///<
    bool is_leaf;
};


/**
 * represents a smelt topology
 */
struct smlt_topology
{
    const char *name;                             ///< name
    struct smlt_topology_node *root;        ///< pointer to the root node
    uint32_t num_nodes;
    struct smlt_topology_node all_nodes[];   ///< array of all nodes
};

//prototypes
static void smlt_topology_create_binary_tree(struct smlt_topology **topology,
                                             uint32_t num_threads);
static void smlt_topology_parse_model(struct smlt_generated_model* model,
                                      struct smlt_topology** topo);
/**
 * @brief initializes the topology subsystem
 *
 * @return SMLT_SUCCESS or error value
 */
errval_t smlt_topology_init(void)
{

    /*
    _tree_prepare();

    if (get_thread_id()==0) {

        shl_barrier_init(&tree_setup_barrier, NULL, get_num_threads());
    }

    _tree_export(qrm_my_name);
     */
    return SMLT_SUCCESS;
}

/**
 * @brief creates a new Smelt topology out of the model parameter
 *
 * @param model         the model input data from the simulator
 * @param length        length of the model input
 * @param name          name of the topology
 * @param ret_topology  returned pointer to the topology
 *
 * @return SMELT_SUCCESS or error value
 *
 * If the model is NULL, then a binary tree will be generated
 */
errval_t smlt_topology_create(struct smlt_generated_model* model,
                              const char *name,
                              struct smlt_topology **ret_topology)
{

    if (model == NULL) {

        *ret_topology = (struct smlt_topology*)
                        smlt_platform_alloc(sizeof(struct smlt_topology)+
                                            sizeof(struct smlt_topology_node)*
                                            smlt_get_num_proc(),
                                            SMLT_DEFAULT_ALIGNMENT, true);
        smlt_topology_create_binary_tree(ret_topology, smlt_get_num_proc());
        (*ret_topology)->num_nodes = smlt_get_num_proc();
    } else {
        *ret_topology = (struct smlt_topology*)
                        smlt_platform_alloc(sizeof(struct smlt_topology)+
                                            sizeof(struct smlt_topology_node)*
                                            model->len,
                                            SMLT_DEFAULT_ALIGNMENT, true);
        smlt_topology_parse_model(model, ret_topology);
        (*ret_topology)->num_nodes = model->len;
    }

    (*ret_topology)->name = name;
    return SMLT_SUCCESS;
}

/**
 * @brief destroys a smelt topology.
 *
 * @param topology  the Smelt topology to destroy
 *
 * @return SMELT_SUCCESS or error vlaue
 */
errval_t smlt_topology_destroy(struct smlt_topology *topology)
{
    return SMLT_SUCCESS;
}

/**
 * \brief Build a binary tree model for the current machine.
 *
 * @param topology      returned pointer to the topology
 * @param num_threads   number of threads/processes in the model
 * @param name          name of the topology
 *
 * @return SMELT_SUCCESS or error value
 */
static void smlt_topology_create_binary_tree(struct smlt_topology **topology,
                                             uint32_t num_threads)
{
    assert (topology!=NULL);
    // Fill model
    (*topology)->root = &((*topology)->all_nodes[0]);

    for (int i = 0; i < num_threads;i++) {

        struct smlt_topology_node* node = &(*topology)->all_nodes[i];
        node->topology = *topology;
        if (i == 0) {
            node->parent = NULL;
            node->node_id = 0;
        } else {
            if ((i % 2) == 0) {
                node->parent = &((*topology)->all_nodes[(i/2)-1]);
                node->array_index = 1;
            } else {
                node->parent = &((*topology)->all_nodes[(i/2)]);
                node->array_index = 0;
            }
        }

        if (i*2+1 < num_threads) {
            node->children = (struct smlt_topology_node**)
                             smlt_platform_alloc(sizeof(struct smlt_topology_node*)*2,
                                                 SMLT_DEFAULT_ALIGNMENT, true);
            node->children[0] = &(*topology)->all_nodes[i*2+1];
            node->node_id = i;
            node->num_children =1;
            if (i*2+2 < num_threads) {
                node->children[1] = &(*topology)->all_nodes[i*2+2];
                node->num_children = 2;
            }
            // TODO set chanel type;
        } else {
            node->children = NULL;
            node->num_children = 0;
            node->node_id = i;
            node->is_leaf = true;
        }
    }
    for (int i = 0; i < num_threads; i++) {
        SMLT_DEBUG(SMLT_DBG__INIT, "Parent of node %d %p \n",
        i, (*topology)->all_nodes[i].parent);
    }
}

/**
 * \brief build topology from model.
 *
 * @param topology      returned pointer to the topology
 *
 * @return SMELT_SUCCESS or error value
 */
static void smlt_topology_parse_model(struct smlt_generated_model* model,
                                      struct smlt_topology** topo)
{

    assert (topo!=NULL);
    // Fill model
    (*topo)->root = &((*topo)->all_nodes[model->root]);
    (*topo)->root->parent = NULL;
    // TODO use node ids instead of cores ids
    for (int x = 0; x < model->len;x++){
        struct smlt_topology_node* node = &((*topo)->all_nodes[x]);

        // find number of children and allocate accordingly
        int max_child = 0;
        for(int y = 0; y < model->len; y++){
            int tmp = model->model[x*model->len+y];
            if ((tmp > max_child) && (tmp != 99)){
               max_child = model->model[x*model->len+y];
            }
        }
        node->node_id = x;
        node->children = (struct smlt_topology_node**)
                             smlt_platform_alloc(sizeof(struct smlt_topology_node*)*
                                                 max_child, SMLT_DEFAULT_ALIGNMENT,
                                                 true);
        // set model
        for(int y = 0; y < model->len; y++){
            int val = model->model[x*(model->len)+y];

            if (val > 0) {
                if (val == 99) {
                    SMLT_DEBUG(SMLT_DBG__INIT,"Parent of %d is %d \n", x, y);
                    node->parent = &((*topo)->all_nodes[y]);
                } else {
                    // TODO add channel type
                    SMLT_DEBUG(SMLT_DBG__INIT,"Child of %d is %d at pos %d \n",
                               x, y, val-1);
                    node->topology = *topo;
                    node->children[val-1] = &((*topo)->all_nodes[y]);
                    node->node_id = x; // TODO change to real node ID
                    (*topo)->all_nodes[y].array_index = val-1;
                }
            }

        }

        node->num_children = max_child;
    }


    for (int i = 0; i < model->len; i++) {
        for (int j = 0; j < model->len; j++) {
            if ((model->leafs[j] == i) && (i != 0)) {
                SMLT_DEBUG(SMLT_DBG__INIT,"%d is a leaf \n", i)
                (*topo)->all_nodes[i].is_leaf = true;
            }
        }
    }
}
/*
 * ===========================================================================
 * topology nodes
 * ===========================================================================
 */


/**
 * @brief returns the first topology node
 *
 * @param topo  the Smelt topology
 *
 * @return Pointer to the smelt topology node
 */
struct smlt_topology_node *smlt_topology_get_first_node(struct smlt_topology *topo)
{
    return topo->all_nodes;
}

/**
 * @brief gets the next topology node in the topology
 *
 * @param node the current topology node
 *
 * @return
 */
struct smlt_topology_node *smlt_topology_node_next(struct smlt_topology_node *node)
{
    return node+1;
}

/**
 * @brief gets the parent topology ndoe
 *
 * @param node the current topology node
 *
 * @return
 */
struct smlt_topology_node *smlt_topology_node_parent(struct smlt_topology_node *node)
{
    return node->parent;
}


/**
 * @brief gets the parent topology ndoe
 *
 * @param node the current topology node
 *
 * @return
 */
struct smlt_topology_node **smlt_topology_node_children(struct smlt_topology_node *node,
                                                        uint32_t* children)
{
    *children = node->num_children;
    return node->children;
}

/**
 * @brief checks if the topology node is the last
 *
 * @param node the topology node
 *
 * @return TRUE if the node is the last, false otherwise
 */
bool smlt_topology_node_is_last(struct smlt_topology_node *node)
{
    return (node == &node->topology->all_nodes[node->topology->num_nodes-1]);
}

/**
 * @brief checks if the topology node is the root
 *
 * @param node the topology node
 *
 * @return TRUE if the node is the root, false otherwise
 */
bool smlt_topology_node_is_root(struct smlt_topology_node *node)
{
    return (node->parent == NULL);
}

/**
 * @brief checks if the topology node is a leaf
 *
 * @param node the topology node
 *
 * @return TRUE if the node is a leaf, false otherwise
 */
bool smlt_topology_node_is_leaf(struct smlt_topology_node *node)
{
    return (node->num_children == 0);
}

/**
 * @brief obtains the child index (the order of the children) from the node
 *
 * @param node  the topology ndoe
 *
 * @return child index
 */
uint32_t smlt_topology_node_get_child_idx(struct smlt_topology_node *node)
{
    return node->array_index;
}

/**
 * @brief gets the number of nodes with the the given idx
 *
 * @param node  the topology node
 *
 * @return numebr of children
 */
uint32_t smlt_topology_node_get_num_children(struct smlt_topology_node *node)
{
    return node->num_children;
}

/**
 * @brief obtainst he associated node id of the topology node
 *
 * @param node  the Smelt topology node
 *
 * @return Smelt node id
 */
smlt_nid_t smlt_topology_node_get_id(struct smlt_topology_node *node)
{
    return node->node_id;
}


/*
 * ===========================================================================
 * queries
 * ===========================================================================
 */


/**
 * @brief obtains the name of the topology
 *
 * @param topo  the Smelt topology
 *
 * @return string representation
 */
const char *smlt_topology_get_name(struct smlt_topology *topo)
{
    return topo->name;
}


/**
 * @brief gets the number of nodes in the topology
 *
 * @param topo  the Smelt topology
 *
 * @return number of nodes
 */
uint32_t smlt_topology_get_num_nodes(struct smlt_topology *topo)
{
    return topo->num_nodes;
}



// --------------------------------------------------
// Data structures - shared
