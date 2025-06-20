#include "mbr.h"
#include <stddef.h>

int mbr_parse(const uint8_t* sector, mbr_part_entry out[4]){
    if(!sector || !out) return 0;
    const mbr_t* mbr = (const mbr_t*)sector;
    if(mbr->signature != 0xAA55) return 0;
    for(int i=0;i<4;i++)
        out[i] = mbr->parts[i];
    return 1;
}
