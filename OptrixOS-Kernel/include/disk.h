#ifndef DISK_H
#define DISK_H
#include <stdint.h>

typedef struct disk {
    uint32_t size;      /* size in bytes */
    uint32_t sector_size;
    uint8_t *data;
} disk_t;

typedef struct partition {
    disk_t *disk;
    uint32_t start;     /* start sector */
    uint32_t sectors;   /* number of sectors */
} partition_t;

disk_t* disk_create(uint32_t size, uint32_t sector_size);
void disk_destroy(disk_t* d);
int disk_read(disk_t* d, uint32_t sector, uint8_t* buf, uint32_t count);
int disk_write(disk_t* d, uint32_t sector, const uint8_t* buf, uint32_t count);

partition_t* partition_create(disk_t* d, uint32_t start, uint32_t sectors);
int partition_read(partition_t* p, uint32_t sector, uint8_t* buf, uint32_t count);
int partition_write(partition_t* p, uint32_t sector, const uint8_t* buf, uint32_t count);

#endif
