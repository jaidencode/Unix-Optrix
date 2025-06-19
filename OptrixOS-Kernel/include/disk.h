#ifndef DISK_H
#define DISK_H
#include <stdint.h>
void ata_init(void);
int ata_read_sector(uint32_t lba, void* buffer);
int ata_write_sector(uint32_t lba, const void* buffer);
#endif
