#ifndef ATA_H
#define ATA_H
#include <stdint.h>

void ata_init(void);
void ata_read(uint32_t lba, uint8_t sector_count, void* buffer);
void ata_write(uint32_t lba, uint8_t sector_count, const void* buffer);

#endif
