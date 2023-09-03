/**
 * @file mbfc.h
 *
 */

#ifndef MBFC_H
#define MBFC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct mbfc_s mbfc_t;

typedef struct {
    void* fp;
    size_t blk_size;
    int cache_num;
    ssize_t (*read_cb)(void* fp, void* buf, size_t nbytes);
    ssize_t (*write_cb)(void* fp, const void* buf, size_t nbytes);
    off_t (*seek_cb)(void* fp, off_t pos, int whence);
} mbfc_param_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void mbfc_param_init(mbfc_param_t* param);

mbfc_t* mbfc_create(const mbfc_param_t* param);

void mbfc_delete(mbfc_t* mbfc);

ssize_t mbfc_read(mbfc_t* mbfc, off_t pos, void* buf, size_t nbyte);

ssize_t mbfc_write(mbfc_t* mbfc, off_t pos, const void* buf, size_t nbyte);

void mbfc_flush(mbfc_t* mbfc);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MBFC_H*/
