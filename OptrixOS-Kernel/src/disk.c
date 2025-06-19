#include "disk.h"
#include "ports.h"

#define ATA_IO_BASE 0x1F0
#define ATA_STATUS (ATA_IO_BASE + 7)
#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRQ 0x08

static void ata_wait_bsy(void){
    while(inb(ATA_STATUS) & ATA_STATUS_BSY);
}
static void ata_wait_drq(void){
    while(!(inb(ATA_STATUS) & ATA_STATUS_DRQ));
}

void ata_init(void){
    ata_wait_bsy();
}

int ata_read_sector(uint32_t lba, void* buffer){
    uint16_t* buf = (uint16_t*)buffer;
    ata_wait_bsy();
    outb(ATA_IO_BASE + 2, 1); // sector count
    outb(ATA_IO_BASE + 3, (uint8_t)lba);
    outb(ATA_IO_BASE + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO_BASE + 5, (uint8_t)(lba >> 16));
    outb(ATA_IO_BASE + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO_BASE + 7, ATA_CMD_READ_PIO);
    ata_wait_bsy();
    ata_wait_drq();
    for(int i=0;i<256;i++)
        buf[i]=inw(ATA_IO_BASE);
    return 0;
}

int ata_write_sector(uint32_t lba, const void* buffer){
    const uint16_t* buf = (const uint16_t*)buffer;
    ata_wait_bsy();
    outb(ATA_IO_BASE + 2, 1);
    outb(ATA_IO_BASE + 3, (uint8_t)lba);
    outb(ATA_IO_BASE + 4, (uint8_t)(lba >> 8));
    outb(ATA_IO_BASE + 5, (uint8_t)(lba >> 16));
    outb(ATA_IO_BASE + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO_BASE + 7, ATA_CMD_WRITE_PIO);
    ata_wait_bsy();
    ata_wait_drq();
    for(int i=0;i<256;i++)
        outw(ATA_IO_BASE, buf[i]);
    ata_wait_bsy();
    return 0;
}
