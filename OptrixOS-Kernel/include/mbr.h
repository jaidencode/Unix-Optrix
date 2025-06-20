#ifndef MBR_H
#define MBR_H
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t boot_ind;
    uint8_t start_chs[3];
    uint8_t type;
    uint8_t end_chs[3];
    uint32_t start_lba;
    uint32_t sectors;
} mbr_part_entry;

typedef struct __attribute__((packed)) {
    uint8_t boot_code[446];
    mbr_part_entry parts[4];
    uint16_t signature;
} mbr_t;

int mbr_parse(const uint8_t* sector, mbr_part_entry out[4]);

#endif
