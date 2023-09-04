/**
 * @file main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#ifdef MBFC_USE_TEST

#include "mbfc.h"
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

#define FILE_PATH "test.dat"
#define FILE_BUFFER_SIZE 1024

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static int gen_file(const char* file_path, const void* buf, size_t size);
static int read_test(const char* file_path, const uint8_t* verify_buf, size_t size);
static int write_test(const char* file_path, const uint8_t* verify_buf, size_t size);

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, const char* argv[])
{
    const char* file_path = FILE_PATH;

    uint8_t file_buf[FILE_BUFFER_SIZE];
    for (int i = 0; i < FILE_BUFFER_SIZE; ++i) {
        file_buf[i] = i;
    }

    if (gen_file(file_path, file_buf, sizeof(file_buf)) < 0) {
        return -1;
    }

    if (read_test(file_path, file_buf, sizeof(file_buf)) < 0) {
        printf("read test failed\n");
    }

    /* reverse file buf */
    for (int i = 0; i < FILE_BUFFER_SIZE; ++i) {
        file_buf[i] = 255 - (i & 0xFF);
    }

    // if (write_test(file_path, file_buf, sizeof(file_buf)) < 0) {
    //     printf("write test failed\n");
    // }

    remove(file_path);

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int gen_file(const char* file_path, const void* buf, size_t size)
{
    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (fd < 0) {
        printf("open %s failed: %d\n", file_path, errno);
        return -1;
    }

    ssize_t bw = write(fd, buf, size);
    if (bw != size) {
        printf("write %zu failed\n", size);
    }
    close(fd);
    return 0;
}

static ssize_t my_read_cb(void* fp, void* buf, size_t nbytes)
{
    return read((int)(intptr_t)fp, buf, nbytes);
}

static ssize_t my_write_cb(void* fp, const void* buf, size_t nbytes)
{
    return write((int)(intptr_t)fp, buf, nbytes);
}

static off_t my_seek_cb(void* fp, off_t pos, int whence)
{
    return lseek((int)(intptr_t)fp, pos, whence);
}

static int read_test(const char* file_path, const uint8_t* verify_buf, size_t size)
{
    int fd = open(file_path, O_RDONLY, 0666);
    if (fd < 0) {
        printf("open %s failed: %d\n", file_path, errno);
        return -1;
    }

    srand(time(NULL));

    mbfc_param_t param;
    mbfc_param_init(&param);
    param.fp = (void*)(intptr_t)fd;
    param.block_size = 16;
    param.cache_num = 16;
    param.read_cb = my_read_cb;
    param.write_cb = my_write_cb;
    param.seek_cb = my_seek_cb;
    mbfc_t* mbfc = mbfc_create(&param);

    int loop = 1000;
    while (loop--) {
        uint8_t rd_buf[FILE_BUFFER_SIZE] = { 0 };

        off_t pos = rand() % FILE_BUFFER_SIZE;
        size_t rd_req_size = rand() % param.block_size;

        ssize_t rd_size = mbfc_read(mbfc, pos, rd_buf, rd_req_size);

        /* File out-of-bounds access */
        if (pos >= FILE_BUFFER_SIZE) {
            if (rd_size <= 0) {
                continue;
            } else {
                printf("mbfc_read failed: %zd\n", rd_size);
                return -1;
            }
        }

        if (rd_size < 0) {
            printf("mbfc_read failed: %zd\n", rd_size);
            return -1;
        }

        if (pos + rd_req_size > FILE_BUFFER_SIZE) {
            rd_req_size = FILE_BUFFER_SIZE - pos;
        }

        if (memcmp(rd_buf, verify_buf + pos, rd_req_size) == 0) {
            printf("memcmp success\n");
        } else {
            printf("memcmp failed\n");
            break;
        }
    }

    printf("cache hit cnt = %d\n", mbfc_get_cache_hit_cnt(mbfc));

    mbfc_delete(mbfc);
    close(fd);

    if (loop > 0) {
        printf("Test failed, loop = %d\n", loop);
    } else {
        printf("Test success\n");
    }

    return 0;
}

static int write_test(const char* file_path, const uint8_t* verify_buf, size_t size)
{
    int fd = open(file_path, O_RDWR, 0666);
    if (fd < 0) {
        printf("open %s failed: %d\n", file_path, errno);
        return -1;
    }

    srand(time(NULL));

    mbfc_param_t param;
    mbfc_param_init(&param);
    param.fp = (void*)(intptr_t)fd;
    param.block_size = 16;
    param.cache_num = 16;
    param.read_cb = my_read_cb;
    param.write_cb = my_write_cb;
    param.seek_cb = my_seek_cb;
    mbfc_t* mbfc = mbfc_create(&param);

    int loop = 1000;
    while (loop--) {
        off_t pos = rand() % FILE_BUFFER_SIZE;
        size_t wr_req_size = rand() % param.block_size;

        if (pos + wr_req_size > FILE_BUFFER_SIZE) {
            wr_req_size = FILE_BUFFER_SIZE - pos;
        }

        ssize_t wr_size = mbfc_write(mbfc, pos, verify_buf + pos, wr_req_size);

        /* File out-of-bounds access */
        if (pos >= FILE_BUFFER_SIZE) {
            if (wr_size <= 0) {
                continue;
            } else {
                printf("mbfc_write failed: %zd\n", wr_size);
                return -1;
            }
        }

        if (wr_size < 0) {
            printf("mbfc_write failed: %zd\n", wr_size);
            return -1;
        }
    }

    printf("cache hit cnt = %d\n", mbfc_get_cache_hit_cnt(mbfc));

    mbfc_flush(mbfc);
    mbfc_delete(mbfc);

    if (loop > 0) {
        printf("Test failed, loop = %d\n", loop);
    } else {
        uint8_t rd_buf[FILE_BUFFER_SIZE] = { 0 };
        ssize_t rd_size = read(fd, rd_buf, sizeof(rd_buf));
        if (rd_size == sizeof(rd_buf) && memcmp(rd_buf, verify_buf, sizeof(rd_buf)) == 0) {
            printf("Test success\n");
        } else {
            printf("memcmp failed\n");
        }
    }

    close(fd);

    return 0;
}

#endif /*MBFC_USE_TEST*/
