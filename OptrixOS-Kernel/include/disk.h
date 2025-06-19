#ifndef DISK_H
#define DISK_H
#include <stdint.h>

void disk_init(void);
int disk_read(uint32_t lba, uint8_t count, void* buf);
int disk_write(uint32_t lba, uint8_t count, const void* buf);

#endif
