#ifndef SSD_H
#define SSD_H
#include <stdint.h>

void ssd_init(void);
int ssd_detect(void);
int ssd_read_sector(uint32_t lba, void* buffer);
int ssd_write_sector(uint32_t lba, const void* buffer);
int ssd_is_present(void);

#endif
