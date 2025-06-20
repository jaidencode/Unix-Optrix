#ifndef DISK_H
#define DISK_H
#include <stdint.h>

typedef struct {
    uint32_t lba_start;
    uint32_t sector_count;
} partition_t;

void disk_init(partition_t* table, int count);
int disk_read(uint32_t lba, uint8_t count, void* buffer);

#endif
