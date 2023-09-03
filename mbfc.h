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
    size_t block_size;
    int cache_num;
    ssize_t (*read_cb)(void* fp, void* buf, size_t nbytes);
    ssize_t (*write_cb)(void* fp, const void* buf, size_t nbytes);
    off_t (*seek_cb)(void* fp, off_t pos, int whence);
} mbfc_param_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize a multi-block file cache parameter structure.
 * @param param: Pointer to the mbfc_param_t structure to be initialized.
 */
void mbfc_param_init(mbfc_param_t* param);

/**
 * Create a multi-block file cache.
 * @param param: Pointer to the configuration parameters for cache creation.
 * @return: Pointer to the created mbfc_t structure.
 */
mbfc_t* mbfc_create(const mbfc_param_t* param);

/**
 * Delete a multi-block file cache and release associated resources.
 * @param mbfc: Pointer to the mbfc_t structure to be deleted.
 */
void mbfc_delete(mbfc_t* mbfc);

/**
 * Read data from a multi-block file cache or the underlying file.
 * @param mbfc: Pointer to the mbfc_t structure.
 * @param pos: The position in the file to start reading from.
 * @param buf: Pointer to the buffer where data will be stored.
 * @param nbyte: The number of bytes to read.
 * @return: The number of bytes read, or -1 on error.
 */
ssize_t mbfc_read(mbfc_t* mbfc, off_t pos, void* buf, size_t nbyte);

/**
 * Write data to a multi-block file cache or the underlying file.
 * @param mbfc: Pointer to the mbfc_t structure.
 * @param pos: The position in the file to start writing to.
 * @param buf: Pointer to the data buffer to be written.
 * @param nbyte: The number of bytes to write.
 * @return: The number of bytes written, or -1 on error.
 */
ssize_t mbfc_write(mbfc_t* mbfc, off_t pos, const void* buf, size_t nbyte);

/**
 * Flush any modified data in the cache to the underlying file.
 * @param mbfc: Pointer to the mbfc_t structure.
 */
void mbfc_flush(mbfc_t* mbfc);

/**
 * Get the cache hit count for the multi-block file cache.
 * @param mbfc: Pointer to the mbfc_t structure.
 * @return: The number of cache hits.
 */
int mbfc_get_cache_hit_cnt(mbfc_t* mbfc);

/**
 * Reset the cache hit count for the multi-block file cache.
 * @param mbfc: Pointer to the mbfc_t structure.
 */
void mbfc_reset_cache_hit_cnt(mbfc_t* mbfc);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*MBFC_H*/
