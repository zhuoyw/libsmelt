/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#include <stdint.h>
#ifndef SMLT_GENERATOR_H_
#define SMLT_GENERATOR_H_ 1


struct smlt_generated_model {
    uint32_t ncores;
    uint32_t last_node;
    uint16_t* model;
    uint32_t* leafs;
};

/**
 * @brief generates a model using the simulator
 *
 * @param cores         an arry of cores that contains the
 *                      cores of the model
 * @param len           length of the cores array
 * @param name          name of the tree topology generated by the
 *                      simulator
 * @param model         result pointer to array of the encoded model
 * @param leafs         result pointer to array of leaf nodes
 * @param last_node     result pointer to last_node 
 *
 * @return              SMLT_SUCCESS if there was no parser/connection 
 *                      error otherwise SMLT_ERR_GENERATOR
 */
errval_t smlt_generate_model(coreid_t* cores, 
                         uint32_t len,
                         char* name,
                         struct smlt_generated_model** model);
/**
 * @brief generates a model from a file storing a json string
 *
 * @param filepath      path to the file
 * @param ncores        number of cores in model
 * @param model         (return value) in struct encoded model
 *                      (last_node, leafs, model)
 *
 * @return              SMLT_SUCCESS if there was no parser error otherwise
 *                      SMLT_ERR_GENERATOR
 */
errval_t smlt_generate_modal_from_file(char* filepath, uint32_t ncores,
                                   struct smlt_generated_model** model);

/**
 * @brief update measurements on the generator
 *        i.e. make new measurements and send them to 
 *        generator service
 *
 * @return SMLT_SUCCESS or SMLT_ERR_GENERATOR if communication
 *         to the generator failed
 */
errval_t smlt_generator_update_measurments(void);
#endif /* SMLT_GENERATOR_H_ */
