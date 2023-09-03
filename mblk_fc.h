/**
 * @file mblk_fc.h
 *
 */

#ifndef MBLK_FC_H
#define MBLK_FC_H

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

typedef struct mblk_fc_s mblk_fc_t;

typedef struct mblk_fc_param_s {
    void* fp;
    size_t blk_size;
    uint32_t cache_num;
    ssize_t (*read_cb)(mblk_fc_t* mblk_fc, void* buf, size_t nbytes);
    ssize_t (*write_cb)(mblk_fc_t* mblk_fc, const void* buf, size_t nbytes);
    off_t (*seek_cb)(mblk_fc_t* mblk_fc, off_t pos, int whence);
} mblk_fc_param_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void mblk_fc_param_init(mblk_fc_param_t* param);

mblk_fc_t* mblk_fc_create(const mblk_fc_param_t* param);

void mblk_fc_delete(mblk_fc_t* mblk_fc_t);

void* mblk_fc_get_fp(mblk_fc_t* mblk_fc);

ssize_t mblk_fc_read(mblk_fc_t* mblk_fc, void* buf, size_t nbyte);

ssize_t mblk_fc_write(mblk_fc_t* mblk_fc, const void* buf, size_t nbyte);

void mblk_fc_seek(mblk_fc_t* mblk_fc, off_t offset, int whence);

void mblk_fc_flush(mblk_fc_t* mblk_fc);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MBLK_FC_H*/
