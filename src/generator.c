/*
 * Copyright (c) 2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */
#include <smlt.h>
#include <smlt_error.h>
#include <smlt_generator.h>
#include <smlt_platform.h>
#include "tree_config.h"
#include <stdio.h> // reading json string from file

/**
 * @brief generates a model using the simulator
 *
 * @param cores         an arry of cores that contains the
 *                      cores of the model
 * @param len           length of the cores array
 * @param name          name of the tree topology generated by the
 *                      simulator
 * @param mode          encoded model (model itself, leafs, last_node)
 *
 */
errval_t smlt_generate_model(coreid_t* cores, uint32_t len,
                         const char* name, struct smlt_generated_model** model)
{
    *model = (struct smlt_generated_model*) smlt_platform_alloc(
                                                sizeof(struct smlt_generated_model),
                                                SMLT_DEFAULT_ALIGNMENT,
                                                true);

    uint32_t len_model;
    int err = smlt_tree_generate(len, cores, name, &((*model)->model),
                                 &((*model)->leafs), &((*model)->root), &len_model);

    printf("Model Generated \n");
    bool all_zeros = true;
    for (unsigned int i = 0; i < len_model; i++) {
        for (unsigned int j = 0; j < len_model; j++) {
            printf("%d ", (*model)->model[i*(len_model)+j]);
            if ((*model)->model[i*(len_model)+j] != 0) {
                all_zeros = false;
            }
        }
        printf("\n");
    }
    (*model)->ncores = len;
    (*model)->len = len_model;

    if (err) {
        return SMLT_ERR_GENERATOR;
    } else if (all_zeros) {
        return SMLT_ERR_GENERATOR;
    } else {
        return SMLT_SUCCESS;
    }
}
/**
 * @brief generates a model from a file storing a json string
 *
 * @param filepath      path to the file
 * @param ncores        number of cores in model
 * @param mode          encoded model (model itself, leafs, last_node)
 *
 */
errval_t smlt_generate_modal_from_file(char* filepath, uint32_t ncores,
                                       struct smlt_generated_model** model)
{

    *model = (struct smlt_generated_model*) smlt_platform_alloc(
                                                sizeof(struct smlt_generated_model),
                                                SMLT_DEFAULT_ALIGNMENT,
                                                true);

    (*model)->ncores = ncores;
    char *json_string;
    uint64_t file_size;
    FILE *file = fopen(filepath, "rb");

    // seek end
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    // read contents
    json_string = (char*) smlt_platform_alloc(file_size * (sizeof(char)),
                                       SMLT_DEFAULT_ALIGNMENT, true);
    uint64_t read = fread(json_string, sizeof(char), file_size, file);
    if (read <= 0) {
       return SMLT_ERR_GENERATOR;
    }

    fclose(file);

    uint32_t len_model;
    int err = smlt_tree_parse(json_string, ncores, &((*model)->model),
                              &((*model)->leafs), &((*model)->root), &len_model);

    (*model)->len = len_model;
    if (err) {
        return SMLT_ERR_GENERATOR;
    } else {
        return SMLT_SUCCESS;
    }
}

/**
 * @brief update measurements on the generator
 *        i.e. make new measurements and send them to
 *        generator service
 *
 * @return SMLT_SUCCESS or SMLT_ERR_GENERATOR if communication
 *         to the generator failed
 */
errval_t smlt_generator_update_measurments(void)
{
    assert(!"NYI");
    return SMLT_SUCCESS;
}
