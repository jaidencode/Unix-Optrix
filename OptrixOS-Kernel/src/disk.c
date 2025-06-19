#include "disk.h"
#include "driver.h"
#include "mem.h"

#define DISK_SIZE (100*1024*1024)
static unsigned char* disk_mem;

static void disk_driver_init(void) {
    disk_mem = mem_alloc(DISK_SIZE);
    if(!disk_mem) disk_mem = (unsigned char*)0;
}

int disk_read(uint32_t lba, uint8_t count, void* buf) {
    if(!disk_mem) return -1;
    uint32_t offset = lba * 512;
    for(uint32_t i=0;i<count*512;i++)
        ((unsigned char*)buf)[i] = disk_mem[offset + i];
    return 0;
}

int disk_write(uint32_t lba, uint8_t count, const void* buf) {
    if(!disk_mem) return -1;
    uint32_t offset = lba * 512;
    for(uint32_t i=0;i<count*512;i++)
        disk_mem[offset + i] = ((const unsigned char*)buf)[i];
    return 0;
}

void disk_init(void) {
    static driver_t drv = {"ramdisk", disk_driver_init, 0};
    driver_register(&drv);
}
