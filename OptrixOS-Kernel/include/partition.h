#ifndef PARTITION_H
#define PARTITION_H
#include <stdint.h>

typedef struct {
    uint8_t boot_indicator;
    uint8_t start_head;
    uint8_t start_sector;
    uint8_t start_cylinder;
    uint8_t system_id;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_cylinder;
    uint32_t start_lba;
    uint32_t sectors;
} partition_entry;

void partition_parse(void* mbr, partition_entry* entries, int max_entries);

#endif
