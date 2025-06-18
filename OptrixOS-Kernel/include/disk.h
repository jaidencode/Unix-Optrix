#ifndef DISK_H
#define DISK_H
#include <stdint.h>
void ata_read_sector(uint32_t lba, uint8_t *buffer);
void ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer);
#endif
