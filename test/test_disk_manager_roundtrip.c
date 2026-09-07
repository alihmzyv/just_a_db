//
// Created by Ali on 08.09.26.
//

#include <assert.h>
#include <disk_manager.h>
#include <string.h>

#include "config.h"

void write_pages(DiskManager* dm) {
    for (uint32_t i = 0; i < 1000; ++i) {
        uint32_t page_num;
        assert(disk_manager_allocate_page(dm, &page_num) == DISK_MANAGER_OK);
        uint8_t buff[PAGE_SIZE] = {0};
        uint8_t *ptr = buff;
        for (int byte_num = 0; byte_num < PAGE_SIZE; byte_num += sizeof(page_num)) {
            memcpy(ptr, &page_num, sizeof(page_num));
            ptr += sizeof(page_num);
        }
        assert(disk_manager_write_page(dm, page_num, buff) == DISK_MANAGER_OK);
    }
}

void read_pages_and_assert(DiskManager * dm) {
    for (uint32_t page_num = 0; page_num < 1000; ++page_num) {
        uint8_t buff_expected[PAGE_SIZE] = {0};
        uint8_t *ptr = buff_expected;
        for (int byte_num = 0; byte_num < PAGE_SIZE; byte_num += sizeof(page_num)) {
            memcpy(ptr, &page_num, sizeof(page_num));
            ptr += sizeof(page_num);
        }

        uint8_t buff_actual[PAGE_SIZE];
        assert(disk_manager_read_page(dm, page_num, buff_actual) == DISK_MANAGER_OK);
        assert(memcmp(buff_expected, buff_actual, PAGE_SIZE) == 0);
    }
}

int main() {
    DiskManager *dm;
    assert(disk_manager_create("test_disk_manager_roundtrip.db", &dm) == DISK_MANAGER_OK);

    write_pages(dm);
    disk_manager_destroy(dm);

    DiskManager *dm2;
    assert(disk_manager_create("test_disk_manager_roundtrip.db", &dm2) == DISK_MANAGER_OK);
    read_pages_and_assert(dm2);
    disk_manager_destroy(dm2);
}
