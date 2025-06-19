#include "disk.h"
#include "ports.h"

#define ATA_STATUS_REG 7
#define ATA_SECTOR_COUNT 2
#define ATA_LBA_LOW 3
#define ATA_LBA_MID 4
#define ATA_LBA_HIGH 5
#define ATA_DRIVE 6
#define ATA_COMMAND 7
#define ATA_DATA 0
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_IDENT_MODEL_START 27
#define ATA_IDENT_MODEL_END 46

static uint16_t ata_io_base = 0x1F0;
#define ATA_REG(r) (ata_io_base + (r))
#define ATA_STATUS ATA_REG(ATA_STATUS_REG)
#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRQ 0x08

static int ata_ssd = 0;

static void ata_wait_bsy(void){
    while(inb(ATA_STATUS) & ATA_STATUS_BSY);
}

static void ata_wait_drq(void){
    while(!(inb(ATA_STATUS) & ATA_STATUS_DRQ));
}

static void ata_identify(void){
    outb(ATA_REG(ATA_SECTOR_COUNT), 0);
    outb(ATA_REG(ATA_LBA_LOW), 0);
    outb(ATA_REG(ATA_LBA_MID), 0);
    outb(ATA_REG(ATA_LBA_HIGH), 0);
    outb(ATA_REG(ATA_DRIVE), 0);
    outb(ATA_REG(ATA_COMMAND), ATA_CMD_IDENTIFY);
    if(inb(ATA_STATUS) == 0) return;
    ata_wait_bsy();
    ata_wait_drq();
    uint16_t id[256];
    for(int i=0;i<256;i++)
        id[i] = inw(ATA_REG(ATA_DATA));
    char model[41];
    int pos = 0;
    for(int w=ATA_IDENT_MODEL_START; w<=ATA_IDENT_MODEL_END; w++){
        model[pos++] = id[w] >> 8;
        model[pos++] = id[w] & 0xFF;
    }
    model[40] = 0;
    ata_ssd = 0;
    for(int i=0; model[i]; i++){
        if(model[i]=='S' && model[i+1]=='S' && model[i+2]=='D'){
            ata_ssd = 1;
            break;
        }
    }
}

int ata_detect(void){
    const uint16_t bases[] = {0x1F0, 0x170};
    for(int i=0;i<2;i++){
        ata_io_base = bases[i];
        uint8_t status = inb(ATA_REG(ATA_STATUS_REG));
        if(status != 0xFF) return 1;
    }
    ata_io_base = 0x1F0;
    return 0;
}

void ata_init(void){
    ata_detect();
    ata_wait_bsy();
    ata_identify();
}

int ata_is_ssd(void){
    return ata_ssd;
}

int ata_read_sector(uint32_t lba, void* buffer){
    uint16_t* buf = (uint16_t*)buffer;
    ata_wait_bsy();
    outb(ATA_REG(ATA_SECTOR_COUNT), 1);
    outb(ATA_REG(ATA_LBA_LOW), (uint8_t)lba);
    outb(ATA_REG(ATA_LBA_MID), (uint8_t)(lba >> 8));
    outb(ATA_REG(ATA_LBA_HIGH), (uint8_t)(lba >> 16));
    outb(ATA_REG(ATA_DRIVE), 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_REG(ATA_COMMAND), ATA_CMD_READ_PIO);
    ata_wait_bsy();
    ata_wait_drq();
    for(int i=0;i<256;i++)
        buf[i]=inw(ATA_REG(ATA_DATA));
    return 0;
}

int ata_write_sector(uint32_t lba, const void* buffer){
    const uint16_t* buf = (const uint16_t*)buffer;
    ata_wait_bsy();
    outb(ATA_REG(ATA_SECTOR_COUNT), 1);
    outb(ATA_REG(ATA_LBA_LOW), (uint8_t)lba);
    outb(ATA_REG(ATA_LBA_MID), (uint8_t)(lba >> 8));
    outb(ATA_REG(ATA_LBA_HIGH), (uint8_t)(lba >> 16));
    outb(ATA_REG(ATA_DRIVE), 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_REG(ATA_COMMAND), ATA_CMD_WRITE_PIO);
    ata_wait_bsy();
    ata_wait_drq();
    for(int i=0;i<256;i++)
        outw(ATA_REG(ATA_DATA), buf[i]);
    ata_wait_bsy();
    return 0;
}
