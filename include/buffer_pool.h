#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include <stdint.h>

typedef struct BufferPool BufferPool;

typedef enum {
    BUFFER_POOL_OK = 0,
    BUFFER_POOL_ERR_IO = -1,
    BUFFER_POOL_ERR_OOM = -2
} BufferPoolStatus;

int buffer_pool_get_page(BufferPool* buffer_pool, uint32_t page_id, void** page);

#endif
