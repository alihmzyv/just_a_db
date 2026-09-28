#include "buffer_pool.h"

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "disk_manager.h"

#define TABLE_MAX_FRAMES 200

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct BufferPool {
    DiskManager* disk_manager;
    void* frames[TABLE_MAX_FRAMES];
} typedef BufferPool;

static int evict(BufferPool* buffer_pool, uint16_t* index) {
    //FIXME: use a proper eviction policy
    free(buffer_pool->frames[0]);
    buffer_pool->frames[0] = NULL;
    *index = 0;
    return 0;
}

static BufferPoolStatus to_buffer_pool_status(DiskManagerStatus disk_manager_status) {
    switch (disk_manager_status) {
        case DISK_MANAGER_OK:
            return BUFFER_POOL_OK;
        default:
            return BUFFER_POOL_ERR_IO;
    }
}

int buffer_pool_get_page(BufferPool* buffer_pool, uint32_t page_id, void** page) {
    pthread_mutex_lock(&lock); //TODO: do fine-grained locking
    BufferPoolStatus status = BUFFER_POOL_OK;
    uint16_t slot = page_id % TABLE_MAX_FRAMES;
    uint32_t page_id_of_slot;
    bool success = false;
    do {
        if (buffer_pool->frames[slot] == NULL) {
            buffer_pool->frames[slot] = malloc(PAGE_SIZE);
            if (buffer_pool->frames[slot] == NULL) {
                status = BUFFER_POOL_ERR_OOM;
                goto cleanup;
            }
            status = to_buffer_pool_status(disk_manager_read_page(buffer_pool->disk_manager, page_id, buffer_pool->frames[slot]));
            if (status != BUFFER_POOL_OK) {
                free(buffer_pool->frames[slot]);
                buffer_pool->frames[slot] = NULL;
                goto cleanup;
            }
            success = true;
            break;
        }
        memcpy(&page_id_of_slot, buffer_pool->frames[slot], sizeof(uint32_t));
        if (page_id_of_slot == page_id) {
            success = true;
            break;
        }
        slot = (slot + 1) % TABLE_MAX_FRAMES;
    } while (slot != page_id % TABLE_MAX_FRAMES);

    if (!success) {  //is full
        evict(buffer_pool, &slot);
        buffer_pool->frames[slot] = malloc(PAGE_SIZE);
        if (buffer_pool->frames[slot] == NULL) {
            status = BUFFER_POOL_ERR_OOM;
            goto cleanup;
        }
        status = to_buffer_pool_status(disk_manager_read_page(buffer_pool->disk_manager, page_id, buffer_pool->frames[slot]));
        if (status != BUFFER_POOL_OK) {
            free(buffer_pool->frames[slot]);
            buffer_pool->frames[slot] = NULL;
            goto cleanup;
        }
    }

    *page = buffer_pool->frames[slot];
    cleanup:
      pthread_mutex_unlock(&lock);
      return status;
}