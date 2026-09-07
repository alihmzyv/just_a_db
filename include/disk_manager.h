#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <stdint.h>

typedef struct DiskManager DiskManager;

int disk_manager_create(const char *filename, DiskManager **out);
int disk_manager_destroy(DiskManager *dm);

int disk_manager_allocate_page(DiskManager *dm, uint32_t *out_page_id);
int disk_manager_num_pages(const DiskManager *dm, uint32_t *out_count);

int disk_manager_read_page(DiskManager *dm, uint32_t page_id, uint8_t *out_buf);
int disk_manager_write_page(DiskManager *dm, uint32_t page_id, const uint8_t *buf);

#endif
