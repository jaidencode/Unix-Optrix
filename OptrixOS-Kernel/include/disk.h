#ifndef DISK_H
#define DISK_H
#include <stddef.h>

#define DISK_SIZE (100*1024*1024)
#define PARTITION_COUNT 2
#define BLOCK_SIZE 512

typedef struct {
    size_t start;
    size_t size;
} partition_t;

void disk_init(void);
partition_t* disk_get_part(int idx);
int disk_part_count(void);
void disk_write_bytes(size_t addr, const void* src, size_t len);
void disk_read_bytes(size_t addr, void* dst, size_t len);
void disk_write_block(partition_t* part, size_t block, const void* src);
void disk_read_block(partition_t* part, size_t block, void* dst);

#endif /* DISK_H */
