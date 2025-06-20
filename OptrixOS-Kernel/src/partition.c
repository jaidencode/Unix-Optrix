#include "partition.h"
#include <stdint.h>

void partition_parse(void* mbr, partition_entry* entries, int max_entries) {
    uint8_t* data = (uint8_t*)mbr;
    for(int i=0;i<max_entries;i++) {
        uint8_t* p = data + 0x1BE + i*16;
        entries[i].boot_indicator = p[0];
        entries[i].start_head = p[1];
        entries[i].start_sector = p[2];
        entries[i].start_cylinder = p[3];
        entries[i].system_id = p[4];
        entries[i].end_head = p[5];
        entries[i].end_sector = p[6];
        entries[i].end_cylinder = p[7];
        entries[i].start_lba = (uint32_t)p[8] | ((uint32_t)p[9]<<8) | ((uint32_t)p[10]<<16) | ((uint32_t)p[11]<<24);
        entries[i].sectors = (uint32_t)p[12] | ((uint32_t)p[13]<<8) | ((uint32_t)p[14]<<16) | ((uint32_t)p[15]<<24);
    }
}
