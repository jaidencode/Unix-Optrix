#include "disk.h"
#include "ports.h"
#include <stddef.h>

static void io_wait(void){
    for(int i=0;i<1000;i++) { __asm__("nop"); }
}

static void ata_wait(void){
    while(inb(0x1F7) & 0x80) { io_wait(); }
}

void ata_read_sectors(uint32_t lba, uint8_t count, uint8_t *buffer){
    ata_wait();
    outb(0x1F6, 0xE0 | ((lba>>24)&0xF));
    outb(0x1F2, count);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba>>8));
    outb(0x1F5, (uint8_t)(lba>>16));
    outb(0x1F7, 0x20); // READ SECTOR
    for(uint8_t s=0; s<count; s++) {
        ata_wait();
        for(int i=0;i<256;i++) {
            uint16_t data = inw(0x1F0);
            buffer[s*512 + i*2] = data & 0xFF;
            buffer[s*512 + i*2 + 1] = data >> 8;
        }
    }
}

void ata_read_sector(uint32_t lba, uint8_t *buffer){
    ata_read_sectors(lba, 1, buffer);
}
