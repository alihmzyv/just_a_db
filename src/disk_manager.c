//
// Created by Ali on 07.09.26.
//
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <config.h>

struct DiskManager {
    int fd;
    uint32_t num_pages;
} typedef DiskManager;

int disk_manager_num_pages(const DiskManager *dm, uint32_t *out_count) {
    *out_count = dm->num_pages;
    return 0;
}

int disk_manager_create(const char *filename, DiskManager **out) {
    int fd = open(filename, O_RDWR |
                    O_CREAT,
                S_IWUSR |
                    S_IRUSR
                );

    if (fd == -1) {
        printf("Error: Could not create or open the file!\n");
        return -1;
    }

    DiskManager* dm = malloc(sizeof(DiskManager));
    dm->fd = fd;
    disk_manager_num_pages(dm, &dm->num_pages);

    *out = dm;

    return 0;
}

int disk_manager_read_page(DiskManager *dm, uint32_t page_id, uint8_t *out_buf) {
    ssize_t bytes_read = pread(dm->fd, out_buf, PAGE_SIZE, page_id * PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) {
        return -1;
    }
    return 0;
}

int disk_manager_write_page(DiskManager *dm, uint32_t page_id, const uint8_t *buf) {
    if (page_id >= dm->num_pages) {
        return -1;
    }
    ssize_t bytes_written = pwrite(dm->fd, buf, PAGE_SIZE, page_id * PAGE_SIZE);
    if (bytes_written != PAGE_SIZE) {
        return -1;
    }
    return 0;
}


int disk_manager_allocate_page(DiskManager *dm, uint32_t *out_page_id) {
    *out_page_id = dm->num_pages++;

    uint8_t buff[PAGE_SIZE] = {0};
    disk_manager_write_page(dm, *out_page_id, buff);
    return 0;
}

int disk_manager_destroy(DiskManager *dm) {
    if (fsync(dm->fd) == -1) {
        return -1;
    }

    if (close(dm->fd) == -1) {
        return -1;
    }

    free(dm);
    return 0;
}

