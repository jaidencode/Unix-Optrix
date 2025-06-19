#include "ata.h"
#include "ports.h"
#include <stdint.h>

#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_SECCOUNT0  0x1F2
#define ATA_LBA0       0x1F3
#define ATA_LBA1       0x1F4
#define ATA_LBA2       0x1F5
#define ATA_HDDEVSEL   0x1F6
#define ATA_COMMAND    0x1F7
#define ATA_STATUS     0x1F7

#define ATA_CMD_READ   0x20
#define ATA_CMD_WRITE  0x30

static void ata_wait_busy(void){
    while(inb(ATA_STATUS) & 0x80);
}

static void ata_wait_drq(void){
    while(!(inb(ATA_STATUS) & 8));
}

void ata_init(void){
    ata_wait_busy();
}

void ata_read(uint32_t lba, uint8_t count, void* buffer){
    ata_wait_busy();
    outb(ATA_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT0, count);
    outb(ATA_LBA0, (uint8_t)lba);
    outb(ATA_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_READ);
    for(int s=0; s<count; s++){
        ata_wait_busy();
        ata_wait_drq();
        uint16_t* buf = (uint16_t*)buffer + s*256;
        for(int i=0;i<256;i++)
            buf[i] = inw(ATA_DATA);
    }
}

void ata_write(uint32_t lba, uint8_t count, const void* buffer){
    ata_wait_busy();
    outb(ATA_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECCOUNT0, count);
    outb(ATA_LBA0, (uint8_t)lba);
    outb(ATA_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, ATA_CMD_WRITE);
    for(int s=0; s<count; s++){
        ata_wait_busy();
        ata_wait_drq();
        const uint16_t* buf = (const uint16_t*)buffer + s*256;
        for(int i=0;i<256;i++)
            outw(ATA_DATA, buf[i]);
    }
    ata_wait_busy();
}
