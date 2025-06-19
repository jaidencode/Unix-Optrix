#include "diskfs.h"
#include "ata.h"
#include "mem.h"
#include <stdint.h>

#define FS_MAGIC 0x4653424F /* 'OBSF' */
#define FS_START_SECTOR 2048
#define FS_META_SECTORS 8
#define FS_MAX_FILES DISKFS_MAX_FILES
#define FS_DATA_START (FS_START_SECTOR + FS_META_SECTORS)

struct meta_entry {
    char name[32];
    uint32_t start;
    uint32_t size;
};

static struct meta_entry metas[FS_MAX_FILES];
static uint32_t next_free_sector = FS_DATA_START;
static int file_count = 0;

static void load_metadata(void){
    uint8_t buf[FS_META_SECTORS*512];
    ata_read(FS_START_SECTOR, FS_META_SECTORS, buf);
    uint32_t* header = (uint32_t*)buf;
    if(header[0] != FS_MAGIC){
        file_count = 0;
        next_free_sector = FS_DATA_START;
        return;
    }
    file_count = header[1];
    if(file_count > FS_MAX_FILES) file_count = FS_MAX_FILES;
    struct meta_entry* on_disk = (struct meta_entry*)(buf + 8);
    for(int i=0;i<file_count;i++)
        metas[i] = on_disk[i];
    next_free_sector = FS_DATA_START;
    for(int i=0;i<file_count;i++){
        uint32_t end = metas[i].start + (metas[i].size + 511)/512;
        if(end > next_free_sector)
            next_free_sector = end;
    }
}

static void save_metadata(void){
    uint8_t buf[FS_META_SECTORS*512];
    for(int i=0;i<FS_META_SECTORS*512;i++) buf[i]=0;
    uint32_t* header = (uint32_t*)buf;
    header[0] = FS_MAGIC;
    header[1] = file_count;
    struct meta_entry* on_disk = (struct meta_entry*)(buf + 8);
    for(int i=0;i<file_count && i<FS_MAX_FILES;i++)
        on_disk[i] = metas[i];
    ata_write(FS_START_SECTOR, FS_META_SECTORS, buf);
}

void diskfs_init(void){
    ata_init();
    load_metadata();
}

static int find_meta(const char* name){
    for(int i=0;i<file_count;i++){
        int j=0;
        while(name[j] && metas[i].name[j] && name[j]==metas[i].name[j]) j++;
        if(name[j]==0 && metas[i].name[j]==0)
            return i;
    }
    return -1;
}

void diskfs_add_entry(fs_entry* e){
    if(file_count >= FS_MAX_FILES) return;
    int idx = find_meta(e->name);
    if(idx >= 0) return;
    idx = file_count++;
    int j=0;
    while(e->name[j] && j<31){ metas[idx].name[j]=e->name[j]; j++; }
    metas[idx].name[j]=0;
    metas[idx].start=0;
    metas[idx].size=0;
    e->disk_start=0;
    e->disk_size=0;
    save_metadata();
}

int diskfs_load_file(fs_entry* e){
    int idx = find_meta(e->name);
    if(idx<0 || metas[idx].size==0)
        return 0;
    e->disk_start = metas[idx].start;
    e->disk_size = metas[idx].size;
    uint8_t* buf = mem_alloc(e->disk_size+1);
    if(!buf) return 0;
    ata_read(e->disk_start, (e->disk_size+511)/512, buf);
    buf[e->disk_size]=0;
    e->content = (char*)buf;
    return 1;
}

void diskfs_write_file(fs_entry* e, const char* data, uint32_t size){
    int idx = find_meta(e->name);
    if(idx<0){
        if(file_count >= FS_MAX_FILES) return;
        idx = file_count++;
        int j=0; while(e->name[j] && j<31){ metas[idx].name[j]=e->name[j]; j++; }
        metas[idx].name[j]=0;
    }
    uint32_t sectors = (size+511)/512;
    uint32_t start = next_free_sector;
    ata_write(start, sectors, data);
    next_free_sector = start + sectors;
    metas[idx].start = start;
    metas[idx].size = size;
    e->disk_start = start;
    e->disk_size = size;
    save_metadata();
}

void diskfs_sync(void){
    /* Persist current metadata state to disk */
    save_metadata();
}

int diskfs_get_count(void){ return file_count; }
const char* diskfs_get_name(int idx){ return (idx>=0 && idx<file_count)?metas[idx].name:""; }
uint32_t diskfs_get_size(int idx){ return (idx>=0 && idx<file_count)?metas[idx].size:0; }
uint32_t diskfs_get_start(int idx){ return (idx>=0 && idx<file_count)?metas[idx].start:0; }
