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
#include <disk_manager.h>

struct DiskManager {
    int fd;
    uint32_t num_pages;
} typedef DiskManager;


int disk_manager_num_pages(const DiskManager *dm, uint32_t *out_count) {
    *out_count = dm->num_pages;
    return DISK_MANAGER_OK;
}

int disk_manager_create(const char *filename, DiskManager **out) {
    int fd = open(filename, O_RDWR |
                    O_CREAT,
                S_IWUSR |
                    S_IRUSR
                );

    if (fd == -1) {
        printf("Error: Could not create or open the file!\n");
        return DISK_MANAGER_ERR_IO;
    }

    DiskManager* dm = malloc(sizeof(DiskManager));
    dm->fd = fd;

    off_t file_length = lseek(fd, 0, SEEK_END);
    dm->num_pages = file_length / PAGE_SIZE;

    *out = dm;

    return DISK_MANAGER_OK;
}

int disk_manager_read_page(DiskManager *dm, uint32_t page_id, uint8_t *out_buf) {
    if (page_id >= dm->num_pages) {
        return DISK_MANAGER_ERR_OUT_OF_RANGE;
    }

    ssize_t bytes_read = pread(dm->fd, out_buf, PAGE_SIZE, page_id * PAGE_SIZE);
    if (bytes_read != PAGE_SIZE) {
        return DISK_MANAGER_ERR_IO;
    }
    return DISK_MANAGER_OK;
}

int disk_manager_write_page(DiskManager *dm, uint32_t page_id, const uint8_t *buf) {
    if (page_id >= dm->num_pages) {
        return DISK_MANAGER_ERR_OUT_OF_RANGE;
    }
    ssize_t bytes_written = pwrite(dm->fd, buf, PAGE_SIZE, page_id * PAGE_SIZE);
    if (bytes_written != PAGE_SIZE) {
        return DISK_MANAGER_ERR_IO;
    }
    return DISK_MANAGER_OK;
}


int disk_manager_allocate_page(DiskManager *dm, uint32_t *out_page_id) {
    *out_page_id = dm->num_pages++;

    uint8_t buff[PAGE_SIZE] = {0};
    int status = disk_manager_write_page(dm, *out_page_id, buff);
    if (status != DISK_MANAGER_OK) {
        dm->num_pages--;
        return status;
    }
    return DISK_MANAGER_OK;
}

int disk_manager_destroy(DiskManager *dm) {
    if (fsync(dm->fd) == -1) {
        return DISK_MANAGER_ERR_IO;
    }

    if (close(dm->fd) == -1) {
        return DISK_MANAGER_ERR_IO;
    }

    free(dm);
    return DISK_MANAGER_OK;
}

