#include "iso9660.h"
#include "ata.h"
#include "fs.h"
#include "mem.h"
#include <stdint.h>

/* Read 2048 byte ISO sector using 512 byte ATA sectors */
static int iso_read_sector2048(uint32_t sector, void* buffer){
    uint8_t* buf = (uint8_t*)buffer;
    for(int i=0;i<4;i++){
        if(ata_read_sectors(sector*4 + i, 1, buf + i*512) != 0)
            return -1;
    }
    return 0;
}

static uint32_t read_le32(const uint8_t* p){
    return p[0] | (p[1]<<8) | (p[2]<<16) | (p[3]<<24);
}

void iso_populate_fs(void){
    uint8_t pvd[2048];
    if(iso_read_sector2048(16, pvd) != 0)
        return;
    if(pvd[0] != 1 || pvd[1] != 'C' || pvd[2] != 'D' || pvd[3] != '0' || pvd[4] != '0' || pvd[5] != '1')
        return;
    const uint8_t* root_rec = pvd + 156;
    uint32_t root_sector = read_le32(root_rec + 2);
    uint32_t root_size = read_le32(root_rec + 10);
    uint8_t* dir = mem_alloc(root_size);
    if(!dir) return;
    for(uint32_t off=0; off<root_size; off+=2048){
        if(iso_read_sector2048(root_sector + off/2048, dir + off) != 0)
            return;
    }
    fs_entry* root = fs_get_root();
    uint32_t pos = 0;
    while(pos < root_size){
        uint8_t len = dir[pos];
        if(len == 0){
            pos = (pos + 2048) & ~0x7FF;
            continue;
        }
        uint8_t flags = dir[pos+25];
        uint8_t name_len = dir[pos+32];
        const char* name = (char*)&dir[pos+33];
        if(name_len == 1 && (name[0] == 0 || name[0] == 1)){
            pos += len;
            continue;
        }
        char fname[32];
        int j=0;
        for(int i=0;i<name_len && j<31;i++){
            char c = name[i];
            if(c == ';') break;
            fname[j++] = c;
        }
        fname[j] = 0;
        if(flags & 2){
            fs_create_dir(root, fname);
        }else{
            uint32_t extent = read_le32(dir+pos+2);
            uint32_t size = read_le32(dir+pos+10);
            char* data = mem_alloc(size);
            if(data){
                for(uint32_t i=0;i<size; i+=2048){
                    iso_read_sector2048(extent + i/2048, data + i);
                }
                fs_entry* f = fs_create_file(root, fname);
                if(f) fs_write_file(f, data);
            }
        }
        pos += len;
    }
}
