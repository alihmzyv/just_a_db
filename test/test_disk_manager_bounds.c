//
// Created by Ali on 08.09.26.
//
#include <assert.h>
#include <disk_manager.h>
#include <unistd.h>
#include <string.h>

#include <config.h>

void write_out_of_bound_page() {
    DiskManager *dm;
    assert(disk_manager_create("test_disk_manager_write_bounds.db", &dm) == DISK_MANAGER_OK);
    uint32_t page_num;

    for (uint32_t i = 0; i < 10; ++i) {
        assert(disk_manager_allocate_page(dm, &page_num) == DISK_MANAGER_OK);
        uint8_t buff[PAGE_SIZE] = {0};
        uint8_t *ptr = buff;
        for (int byte_num = 0; byte_num < PAGE_SIZE; byte_num += sizeof(page_num)) {
            memcpy(ptr, &page_num, sizeof(page_num));
            ptr += sizeof(page_num);
        }
        disk_manager_write_page(dm, page_num, buff);
    }

    uint8_t buff[1];
    assert(disk_manager_write_page(dm, page_num + 1, buff) == DISK_MANAGER_ERR_OUT_OF_RANGE);
    disk_manager_destroy(dm);
}

void read_out_of_bound_page() {
    DiskManager* dm;
    assert(disk_manager_create("test_disk_managerr_read_bounds.db", &dm) == DISK_MANAGER_OK);

    uint8_t buff_actual[1];
    assert(disk_manager_read_page(dm, 10, buff_actual) == DISK_MANAGER_ERR_OUT_OF_RANGE);
    disk_manager_destroy(dm);
}

int main() {
    write_out_of_bound_page();
    read_out_of_bound_page();
}
