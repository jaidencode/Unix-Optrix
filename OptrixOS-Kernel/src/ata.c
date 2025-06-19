#include "ata.h"
#include "ports.h"

static int ata_wait(void) {
    for(int i=0;i<100000;i++) {
        uint8_t status = inb(0x1F7);
        if(!(status & 0x80) && (status & 0x08))
            return 0;
    }
    return -1;
}

int ata_read_sectors(uint32_t lba, uint8_t count, void* buffer) {
    uint16_t* buf = (uint16_t*)buffer;
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, count);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);

    for(int s=0; s<count; s++) {
        if(ata_wait())
            return -1;
        for(int i=0;i<256;i++) {
            buf[i] = inw(0x1F0);
        }
        buf += 256;
    }
    return 0;
}
