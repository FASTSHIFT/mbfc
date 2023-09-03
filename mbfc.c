/**
 * @file mbfc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "mbfc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#define MBFC_AGE_MAX 100
#define MBFC_AGE_INC(cache) ((cache)->age < MBFC_AGE_MAX ? (cache)->age++ : 0)

#define MBFC_ALIGN(value, align) ((value) & ~((align)-1))

#define MBFC_MALLOC(size) malloc(size)
#define MBFC_FREE(ptr) free(ptr)
#define MBFC_ASSERT(expr) assert(expr)
#define MBFC_LOG_INFO(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define MBFC_LOG_WARN(fmt, ...) printf(fmt, ##__VA_ARGS__)

/**********************
 *      TYPEDEFS
 **********************/

typedef struct mbfc_cache_s {
    off_t pos;
    uint8_t* buf;
    size_t size;
    int age;
} mbfc_cache_t;

struct mbfc_s {
    mbfc_param_t param;
    mbfc_cache_t* cache_arr;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

static mbfc_cache_t* mbfc_find_block(mbfc_t* mbfc, off_t pos);
static mbfc_cache_t* mbfc_load_block(mbfc_t* mbfc, off_t pos);
static void mbfc_age_dec_all(mbfc_t* mbfc);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void mbfc_param_init(mbfc_param_t* param)
{
    MBFC_ASSERT(param != NULL);
    memset(param, 0, sizeof(mbfc_param_t));
    param->blk_size = 1024;
    param->cache_num = 8;
}

mbfc_t* mbfc_create(const mbfc_param_t* param)
{
    MBFC_ASSERT(param != NULL);
    MBFC_ASSERT(param->fp != NULL);
    MBFC_ASSERT(param->blk_size > 0);
    MBFC_ASSERT(param->cache_num > 0);
    MBFC_ASSERT(param->read_cb != NULL);
    MBFC_ASSERT(param->write_cb != NULL);
    MBFC_ASSERT(param->seek_cb != NULL);

    mbfc_t* mbfc = MBFC_MALLOC(sizeof(mbfc_t));
    MBFC_ASSERT(mbfc != NULL);
    memset(mbfc, 0, sizeof(mbfc_t));
    mbfc->param = *param;

    mbfc->cache_arr = MBFC_MALLOC(sizeof(mbfc_cache_t) * param->cache_num);
    MBFC_ASSERT(mbfc->cache_arr != NULL);
    memset(mbfc->cache_arr, 0, sizeof(mbfc_cache_t) * param->cache_num);

    for (int i = 0; i < param->cache_num; i++) {
        mbfc->cache_arr[i].buf = MBFC_MALLOC(param->blk_size);
        MBFC_ASSERT(mbfc->cache_arr[i].buf != NULL);
        memset(mbfc->cache_arr[i].buf, 0, param->blk_size);
    }

    return mbfc;
}

void mbfc_delete(mbfc_t* mbfc)
{
    MBFC_ASSERT(mbfc != NULL);

    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];
        MBFC_LOG_INFO("%s: free cache[%d], pos %d, age = %d\n", __func__, i, (int)cache->pos, cache->age);
        MBFC_FREE(cache->buf);
    }

    MBFC_FREE(mbfc->cache_arr);
    MBFC_FREE(mbfc);
}

ssize_t mbfc_read(mbfc_t* mbfc, off_t pos, void* buf, size_t nbyte)
{
    MBFC_ASSERT(mbfc != NULL);

    if (!buf || !nbyte) {
        return 0;
    }

    ssize_t ret = -1;
    ssize_t remain = nbyte;
    uint8_t* cur = buf;

    while (remain > 0) {
        mbfc_cache_t* cache = mbfc_find_block(mbfc, pos);

        if (cache) {
            MBFC_LOG_INFO("%s: cache hit pos %d, age = %d\n", __func__, (int)pos, cache->age);
        } else {
            MBFC_LOG_INFO("%s: cache miss %d\n", __func__, (int)pos);
            cache = mbfc_load_block(mbfc, pos);
            if (!cache) {
                MBFC_LOG_WARN("%s: load block failed\n", __func__);
                break;
            }
            mbfc_age_dec_all(mbfc);
        }

        MBFC_AGE_INC(cache);

        size_t cp = cache->size - (pos - cache->pos);
        cp = (cp > remain) ? remain : cp;
        memcpy(cur, cache->buf + (pos - cache->pos), cp);
        pos += cp;
        cur += cp;
        remain -= cp;
        ret += cp;
    }

    return ret;
}

ssize_t mbfc_write(mbfc_t* mbfc, off_t pos, const void* buf, size_t nbyte)
{
    MBFC_ASSERT(mbfc != NULL);

    return -1;
}

void mbfc_flush(mbfc_t* mbfc)
{
    MBFC_ASSERT(mbfc != NULL);

    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age <= 0) {
            continue;
        }

        off_t offset = mbfc->param.seek_cb(mbfc, cache->pos, SEEK_SET);

        if (offset < 0) {
            MBFC_LOG_WARN("%s: cache[%d] seek failed\n", __func__, i);
            continue;
        }

        ssize_t wr = mbfc->param.write_cb(mbfc, cache->buf, cache->size);

        if (wr != cache->size) {
            MBFC_LOG_WARN("%s: cache[%d] write %zd != %zu, failed\n",
                __func__, i, wr, cache->size);
            continue;
        }

        cache->age = 0;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static mbfc_cache_t* mbfc_find_block(mbfc_t* mbfc, off_t pos)
{
    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age <= 0) {
            continue;
        }

        pos = MBFC_ALIGN(pos, mbfc->param.blk_size);

        if (cache->pos == pos) {
            return cache;
        }
    }

    return NULL;
}

static mbfc_cache_t* mbfc_get_reuse(mbfc_t* mbfc)
{
    int min_age = MBFC_AGE_MAX;
    mbfc_cache_t* min_age_cache = NULL;

    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age <= 0) {
            return cache;
        }

        if (cache->age < min_age) {
            min_age = cache->age;
            min_age_cache = cache;
        }
    }

    return min_age_cache;
}

static void mbfc_age_dec_all(mbfc_t* mbfc)
{
    for (int i = 0; i < mbfc->param.cache_num; i++) {
        mbfc_cache_t* cache = &mbfc->cache_arr[i];

        if (cache->age > 0) {
            cache->age--;
        }
    }
}

static mbfc_cache_t* mbfc_load_block(mbfc_t* mbfc, off_t pos)
{
    mbfc_cache_t* cache = mbfc_get_reuse(mbfc);
    MBFC_ASSERT(cache != NULL);

    pos = MBFC_ALIGN(pos, mbfc->param.blk_size);
    cache->pos = pos;
    cache->age = 0;

    off_t offset = mbfc->param.seek_cb(mbfc->param.fp, pos, SEEK_SET);

    if (offset < 0) {
        MBFC_LOG_WARN("%s: cache seek %d failed\n", __func__, (int)pos);
        return NULL;
    }

    ssize_t rd = mbfc->param.read_cb(mbfc->param.fp, cache->buf, mbfc->param.blk_size);

    if (rd <= 0) {
        MBFC_LOG_WARN("%s: cache read %zd != %zu, failed\n",
            __func__, rd, mbfc->param.blk_size);
        return NULL;
    }

    cache->age = 1;
    cache->size = rd;

    MBFC_LOG_INFO("%s: cache load pos: %d, size = %zu\n", __func__, (int)pos, cache->size);

    return cache;
}
