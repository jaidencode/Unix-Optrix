#include "fs.h"
#include "mem.h"
#include "disk.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    char name[32];
    uint32_t lba;
    uint32_t size;
} disk_file;

static disk_file* disk_files = NULL;
static uint32_t disk_file_count = 0;
static fs_entry root_dir;

static size_t fs_strlen(const char* s){ size_t l=0; while(s[l]) l++; return l; }

static fs_entry* alloc_entry(const char*name,int is_dir){
    fs_entry* e = mem_alloc(sizeof(fs_entry));
    if(!e) return NULL;
    size_t len=fs_strlen(name)+1;
    e->name = mem_alloc(len);
    if(!e->name) return NULL;
    for(size_t i=0;i<len;i++) e->name[i]=name[i];
    e->is_dir=is_dir;
    e->parent=NULL;
    e->child=NULL;
    e->sibling=NULL;
    e->content=NULL;
    e->lba=0;
    e->size=0;
    return e;
}

static void add_child(fs_entry*parent,fs_entry*child){
    child->parent=parent;
    child->sibling=parent->child;
    parent->child=child;
}

fs_entry* fs_get_root(void){
    return &root_dir;
}

fs_entry* fs_create_dir(fs_entry* dir, const char* name){
    if(!dir || !dir->is_dir)
        return NULL;
    /* simple heap based directory creation */
    fs_entry* e = alloc_entry(name, 1);
    if(!e)
        return NULL;
    add_child(dir, e);
    return e;
}
fs_entry* fs_create_file(fs_entry* dir, const char* name){
    fs_entry* e=alloc_entry(name,0);
    if(!e) return NULL;
    add_child(dir,e);
    return e;
}
fs_entry* fs_find_entry(fs_entry* dir, const char* name){
    for(fs_entry*c=dir->child;c;c=c->sibling){
        size_t i=0;while(c->name[i] && name[i] && c->name[i]==name[i]) i++; if(c->name[i]==0 && name[i]==0) return c;
    }
    return NULL;
}
fs_entry* fs_find_subdir(fs_entry* dir,const char* name){
    if(!dir || !dir->is_dir)
        return NULL;
    for(fs_entry*c=dir->child;c;c=c->sibling){
        size_t i=0;while(c->name[i] && name[i] && c->name[i]==name[i]) i++;
        if(c->name[i]==0 && name[i]==0 && c->is_dir)
            return c;
    }
    return NULL;
}

int fs_delete_entry(fs_entry* dir,const char* name){
    if(!dir || !dir->is_dir)
        return 0;
    fs_entry* prev=NULL;
    for(fs_entry*c=dir->child;c;c=c->sibling){
        size_t i=0;while(c->name[i] && name[i] && c->name[i]==name[i]) i++;
        if(c->name[i]==0 && name[i]==0){
            if(prev) prev->sibling=c->sibling; else dir->child=c->sibling;
            return 1;
        }
        prev=c;
    }
    return 0;
}

void fs_write_file(fs_entry* file,const char* text){
    if(!file || file->is_dir)
        return;
    size_t len = fs_strlen(text);
    if(len>255) len=255;
    char* buf = mem_alloc(len+1);
    if(!buf) return;
    for(size_t i=0;i<len;i++) buf[i]=text[i];
    buf[len]='\0';
    file->content = buf;
    file->size = len;
}

const char* fs_read_file(fs_entry* file){
    if(!file||file->is_dir) return "";
    if(file->content)
        return file->content;
    uint32_t sectors = (file->size + 511) / 512;
    char* buf = mem_alloc(sectors*512 + 1);
    if(!buf) return "";
    for(uint32_t i=0;i<sectors;i++)
        ata_read_sector(file->lba + i, buf + i*512);
    buf[file->size] = 0;
    return buf;
}

void fs_init(void){
    ata_init();

    unsigned char first[512];
    ata_read_sector(1, first);
    uint32_t* meta = (uint32_t*)first;
    disk_file_count = meta[0];
    uint32_t root_sectors = meta[1];

    unsigned int entry_bytes = disk_file_count * sizeof(disk_file);
    unsigned int table_bytes = 12 + entry_bytes;
    unsigned int sectors = root_sectors;

    unsigned char* buf = mem_alloc(sectors * 512);
    if(!buf) return;

    for(unsigned int i=0;i<sectors;i++){
        if(i==0){
            for(int j=0;j<512;j++) buf[j] = first[j];
        }else{
            ata_read_sector(1+i, buf + i*512);
        }
    }

    disk_files = (disk_file*)(buf + 12);

    root_dir.name="/";
    root_dir.is_dir=1;
    root_dir.parent=NULL;
    root_dir.child=NULL;
    root_dir.sibling=NULL;
    root_dir.content=NULL;
    root_dir.lba=0;
    root_dir.size=0;

    for(uint32_t i=0;i<disk_file_count;i++){
        const char* name = disk_files[i].name;
        fs_entry* dir = &root_dir;
        char part[32];
        size_t pi=0;
        for(size_t j=0;;j++){
            char c = name[j];
            if(c=='/' || c==0){
                part[pi]=0;
                if(c==0){
                    fs_entry* f = fs_create_file(dir, part);
                    if(f){
                        f->lba = disk_files[i].lba;
                        f->size = disk_files[i].size;
                    }
                    break;
                }else{
                    fs_entry* sub = fs_find_subdir(dir, part);
                    if(!sub)
                        sub = fs_create_dir(dir, part);
                    dir = sub;
                    pi = 0;
                }
            }else if(pi < 31){
                part[pi++] = c;
            }
        }
    }
}
