#include "disk.h"

static partition_t partitions[4];
static int part_count = 0;

void disk_init(partition_t* table, int count){
    if(count > 4) count = 4;
    for(int i=0;i<count;i++)
        partitions[i] = table[i];
    part_count = count;
}

/* BIOS LBA read using INT 13h AH=42h. Only works in real mode. */
int disk_read(uint32_t lba, uint8_t count, void* buffer){
    struct dap {
        uint8_t size;
        uint8_t reserved;
        uint16_t count;
        uint16_t offset;
        uint16_t segment;
        uint32_t lba;
        uint32_t lba_hi;
    } __attribute__((packed)) d;
    d.size = 16;
    d.reserved = 0;
    d.count = count;
    d.offset = (uint16_t)((uint32_t)buffer & 0xF);
    d.segment = ((uint32_t)buffer >> 4) & 0xFFFF;
    d.lba = lba;
    d.lba_hi = 0;
    int status;
    __asm__(
        "push %%es\n"
        "mov %3, %%ax\n"
        "mov %%ax, %%es\n"
        "mov $0x4200, %%ax\n"
        "int $0x13\n"
        "pop %%es\n"
        : "=a"(status)
        : "S"(&d), "d"(0x80), "r"(d.segment)
        : "memory" );
    return (status & 0xFF00) == 0;
}
