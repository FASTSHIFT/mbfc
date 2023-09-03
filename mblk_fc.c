/**
 * @file mblk_fc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "mblk_fc.h"
#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#define MBLK_FC_MALLOC(size) malloc(size)
#define MBLK_FC_FREE(ptr) free(ptr)

/**********************
 *      TYPEDEFS
 **********************/

typedef struct mblk_fc_cache_s {
    void* buf;
    size_t size;
    int life;
} mblk_fc_cache_t;

struct mblk_fc_s {
    mblk_fc_param_t param;
    mblk_fc_cache_t* cache_arr;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/
