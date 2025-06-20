#include "disk.h"
#include "mem.h"
#include <stdint.h>

static unsigned char disk_data[DISK_SIZE];
static partition_t partitions[PARTITION_COUNT];

void disk_init(void){
    /* simple two partition layout: half and half */
    size_t half = DISK_SIZE/2;
    partitions[0].start = 0;
    partitions[0].size = half;
    partitions[1].start = half;
    partitions[1].size = DISK_SIZE - half;
}

partition_t* disk_get_part(int idx){
    if(idx < 0 || idx >= PARTITION_COUNT)
        return 0;
    return &partitions[idx];
}

int disk_part_count(void){
    return PARTITION_COUNT;
}

void disk_write_bytes(size_t addr, const void* src, size_t len){
    if(addr + len > DISK_SIZE) return;
    const unsigned char* s = src;
    for(size_t i=0;i<len;i++)
        disk_data[addr+i] = s[i];
}

void disk_read_bytes(size_t addr, void* dst, size_t len){
    if(addr + len > DISK_SIZE) return;
    unsigned char* d = dst;
    for(size_t i=0;i<len;i++)
        d[i] = disk_data[addr+i];
}

void disk_write_block(partition_t* part, size_t block, const void* src){
    if(!part) return;
    size_t addr = part->start + block * BLOCK_SIZE;
    disk_write_bytes(addr, src, BLOCK_SIZE);
}

void disk_read_block(partition_t* part, size_t block, void* dst){
    if(!part) return;
    size_t addr = part->start + block * BLOCK_SIZE;
    disk_read_bytes(addr, dst, BLOCK_SIZE);
}
