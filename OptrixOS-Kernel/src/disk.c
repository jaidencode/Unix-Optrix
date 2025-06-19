#include "disk.h"
#include "ports.h"
#include "driver.h"

#define ATA_IO 0x1F0
#define ATA_CTRL 0x3F6

static int ata_wait(void) {
    for(int i=0;i<100000;i++) {
        uint8_t status = inb(ATA_IO + 7);
        if(!(status & 0x80) && (status & 0x08))
            return 0;
    }
    return -1;
}

int disk_read_sector(uint32_t lba, uint8_t *buf) {
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 2, 1);
    outb(ATA_IO + 3, lba & 0xFF);
    outb(ATA_IO + 4, (lba >> 8) & 0xFF);
    outb(ATA_IO + 5, (lba >> 16) & 0xFF);
    outb(ATA_IO + 7, 0x20); // READ SECTORS
    if(ata_wait() < 0) return -1;
    for(int i=0;i<256;i++) {
        uint16_t w = inw(ATA_IO);
        buf[i*2] = w & 0xFF;
        buf[i*2+1] = (w >> 8) & 0xFF;
    }
    return 0;
}

int disk_write_sector(uint32_t lba, const uint8_t *buf) {
    outb(ATA_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_IO + 2, 1);
    outb(ATA_IO + 3, lba & 0xFF);
    outb(ATA_IO + 4, (lba >> 8) & 0xFF);
    outb(ATA_IO + 5, (lba >> 16) & 0xFF);
    outb(ATA_IO + 7, 0x30); // WRITE SECTORS
    if(ata_wait() < 0) return -1;
    for(int i=0;i<256;i++) {
        uint16_t w = buf[i*2] | (buf[i*2+1] << 8);
        outw(ATA_IO, w);
    }
    outb(ATA_CTRL, 0);
    ata_wait();
    return 0;
}

static void disk_drv_init(void) {}

static driver_t drv = {"disk", disk_drv_init, 0};

void disk_init(void) {
    driver_register(&drv);
}
