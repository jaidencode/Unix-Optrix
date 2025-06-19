#ifndef DISK_H
#define DISK_H
#include <stdint.h>
void ata_init(void);
void ata_select(uint16_t base, int slave);
int ata_detect(void);
int ata_read_sector(uint32_t lba, void* buffer);
int ata_write_sector(uint32_t lba, const void* buffer);
#endif
