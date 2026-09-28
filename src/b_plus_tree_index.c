//
// Created by Ali on 16.09.26.
//
#include <stdlib.h>

#include <pthread.h>

#include "buffer_pool.h"

#include "b_plus_tree_index.h"

#include <stdbool.h>
#include <string.h>
#include <secure/_string.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

typedef enum {
    INTERNAL = 0,
    LEAF = 1
} PageType;

const uint32_t PAGE_TYPE_SIZE = 1;
const uint32_t KEY_NUM_SIZE = sizeof(uint16_t);
const uint32_t PAGE_ID_SIZE = sizeof(uint32_t);
const uint32_t KEY_SIZE_SIZE = sizeof(uint16_t);
const uint32_t ROW_OFFSET_SIZE = sizeof(uint16_t);
const char HAS_NEXT_PAGE_SIZE = sizeof(char);

struct CurrentTupleId {
    uint32_t page_id;
    uint16_t slot;
} typedef CurrentTupleId;

struct BPlusTreeIndex {
    BufferPool *buffer_pool;
} typedef BPlusTreeIndex;

static BPlusTreeStatus to_b_plus_tree_index_status(BufferPoolStatus buffer_pool_status) {
    switch (buffer_pool_status) {
        case BUFFER_POOL_OK:
            return B_PLUS_TREE_OK;
        default:
            return B_PLUS_TREE_ERR_IO;
    }
}

int get_ctid(BPlusTreeIndex *index, void *value, u_int64_t bytes, CurrentTupleId **ctid, uint32_t *match_num) {
    *match_num = 0;
    pthread_mutex_lock(&lock);
    BPlusTreeStatus status = B_PLUS_TREE_OK;
    CurrentTupleId *ctid_index = NULL;

    void *page = NULL;
    status = to_b_plus_tree_index_status(buffer_pool_get_page(index->buffer_pool, 0, &page));
    if (status != B_PLUS_TREE_OK) {
        goto cleanup;
    }

    uint32_t root_page_id;
    memcpy(&root_page_id, page, sizeof(root_page_id));
    status = to_b_plus_tree_index_status(buffer_pool_get_page(index->buffer_pool, root_page_id, &page));
    if (status != B_PLUS_TREE_OK) {
        goto cleanup;
    }

    char page_type;
    memcpy(&page_type, page, PAGE_TYPE_SIZE);

    uint32_t page_id;
    uint16_t key_num;
    char *key_begin;
    uint16_t key_size;
    while (page_type != LEAF) {
        memcpy(&key_num, page + PAGE_TYPE_SIZE, KEY_NUM_SIZE);

        key_begin = page + PAGE_TYPE_SIZE + KEY_NUM_SIZE;

        int i;
        for (i = 0; i < key_num; ++i) {
            key_begin += PAGE_ID_SIZE + KEY_SIZE_SIZE;
            if (i != 0) {
                key_begin += key_size;
            }
            memcpy(&key_size, key_begin - KEY_SIZE_SIZE, KEY_SIZE_SIZE);
            if (memcmp(value, key_begin, bytes < key_size ? bytes : key_size) <= 0) {
                memcpy(&page_id, key_begin - KEY_SIZE_SIZE - PAGE_ID_SIZE, PAGE_ID_SIZE);
                status = to_b_plus_tree_index_status(buffer_pool_get_page(index->buffer_pool, page_id, &page));
                if (status != B_PLUS_TREE_OK) {
                    goto cleanup;
                }

                memcpy(&page_type, page, PAGE_TYPE_SIZE);
                break;
            }
        }

        if (i == key_num) {
            memcpy(&page_id, key_begin + key_size, PAGE_ID_SIZE);
            status = to_b_plus_tree_index_status(buffer_pool_get_page(index->buffer_pool, page_id, &page));
            if (status != B_PLUS_TREE_OK) {
                goto cleanup;
            }

            memcpy(&page_type, page, PAGE_TYPE_SIZE);
        }
    }

    bool page_finished = true;

    int size_of_arr = 10;
    ctid_index = malloc(size_of_arr * sizeof(CurrentTupleId));
    if (ctid_index == NULL) {
        status = B_PLUS_TREE_ERR_OOM;
        goto cleanup;
    }

    while (1) {
        memcpy(&key_num, page + PAGE_TYPE_SIZE, KEY_NUM_SIZE);
        key_begin = page + PAGE_TYPE_SIZE + KEY_NUM_SIZE + KEY_SIZE_SIZE;
        memcpy(&key_size, key_begin - KEY_SIZE_SIZE, KEY_SIZE_SIZE);

        for (int i = 0; i < key_num; ++i) {
            int comp_result = memcmp(value, key_begin, bytes < key_size ? bytes : key_size);
            if (comp_result == 0) {
                uint32_t row_page_id;
                memcpy(&row_page_id, key_begin + key_size, PAGE_ID_SIZE);
                uint16_t slot;
                memcpy(&slot, key_begin + key_size + PAGE_ID_SIZE, ROW_OFFSET_SIZE);
                if (size_of_arr == *match_num) {
                    size_of_arr = 2 * size_of_arr;
                    CurrentTupleId* to_free = ctid_index - *match_num;
                    CurrentTupleId* temp = ctid_index - *match_num;
                    ctid_index = malloc(size_of_arr * sizeof(CurrentTupleId));
                    if (ctid_index == NULL) {
                        status = B_PLUS_TREE_ERR_OOM;
                        goto cleanup;
                    }
                    for (int j = 0; j < *match_num; ++j) {
                        *ctid_index = *temp;
                        ctid_index++;
                        temp++;
                    }
                    free(to_free);
                }
                ctid_index->page_id = row_page_id;
                ctid_index->slot = slot;
                ctid_index++;
                *match_num = *match_num + 1;
            } else if (comp_result < 0) {
                page_finished = false;
                break;
            }
            key_begin += key_size + PAGE_ID_SIZE + ROW_OFFSET_SIZE + KEY_SIZE_SIZE;
            memcpy(&key_size, key_begin - KEY_SIZE_SIZE, KEY_SIZE_SIZE);
        }

        if (page_finished) {
            char has_next_page;
            memcpy(&has_next_page, key_begin - KEY_SIZE_SIZE, HAS_NEXT_PAGE_SIZE);

            if (has_next_page) {
                uint32_t next_page_id;
                memcpy(&next_page_id, key_begin - KEY_SIZE_SIZE + HAS_NEXT_PAGE_SIZE, PAGE_ID_SIZE);
                status = to_b_plus_tree_index_status(buffer_pool_get_page(index->buffer_pool, next_page_id, &page));
                if (status != B_PLUS_TREE_OK) {
                    goto cleanup;
                }
            } else {
                break;
            }
        } else {
            break;
        }
    }

    *ctid = ctid_index - *match_num;

cleanup:
    pthread_mutex_unlock(&lock);
    if (status != B_PLUS_TREE_OK && ctid_index != NULL) {
        free(ctid_index);
    }
    return status;
}
