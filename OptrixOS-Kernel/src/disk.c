#include "disk.h"
#include "ports.h"

#define ATA_IO 0x1F0
#define ATA_CTRL 0x3F6

static void ata_wait_bsy(void){ while(inb(ATA_IO + 7) & 0x80); }
static void ata_wait_drq(void){ while(!(inb(ATA_IO + 7) & 8)); }

int disk_read_sector(uint32_t lba, uint8_t* buf){
    ata_wait_bsy();
    outb(ATA_CTRL, 0x00);
    outb(ATA_IO + 2, 1);
    outb(ATA_IO + 3, (uint8_t)lba);
    outb(ATA_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 7, 0x20);
    ata_wait_bsy();
    if(!(inb(ATA_IO + 7) & 8)) return 0;
    for(int i=0;i<256;i++){
        uint16_t w = inw(ATA_IO);
        buf[i*2] = w & 0xFF;
        buf[i*2+1] = (w >> 8) & 0xFF;
    }
    return 1;
}

int disk_write_sector(uint32_t lba, const uint8_t* buf){
    ata_wait_bsy();
    outb(ATA_CTRL, 0x00);
    outb(ATA_IO + 2, 1);
    outb(ATA_IO + 3, (uint8_t)lba);
    outb(ATA_IO + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO + 5, (uint8_t)(lba >> 16));
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 7, 0x30);
    ata_wait_bsy();
    for(int i=0;i<256;i++){
        uint16_t w = buf[i*2] | ((uint16_t)buf[i*2+1] << 8);
        outw(ATA_IO, w);
    }
    outb(ATA_IO + 7, 0xE7);
    ata_wait_bsy();
    return 1;
}
