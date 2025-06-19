#include "ssd.h"
#include "disk.h"

void ssd_init(void){
    ata_init();
}

int ssd_detect(void){
    return ata_detect();
}

int ssd_read_sector(uint32_t lba, void* buffer){
    return ata_read_sector(lba, buffer);
}

int ssd_write_sector(uint32_t lba, const void* buffer){
    return ata_write_sector(lba, buffer);
}

int ssd_is_present(void){
    return ata_detect();
}
