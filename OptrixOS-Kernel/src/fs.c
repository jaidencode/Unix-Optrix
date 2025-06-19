#include "fs.h"
#include "mem.h"
#include "disk.h"
#include <stddef.h>
#include <stdint.h>

extern uint32_t end; /* provided by linker */
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
    e->embedded=0;
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

fs_entry* fs_find_path(const char* path){
    if(!path || !*path) return fs_get_root();
    fs_entry* dir = fs_get_root();
    char part[32];
    size_t pi = 0;
    for(size_t i=0;;i++){
        char c = path[i];
        if(c=='/' || c==0){
            part[pi]=0;
            if(part[0]){
                fs_entry* next = fs_find_entry(dir, part);
                if(!next) return NULL;
                dir = next;
            }
            pi = 0;
            if(c==0) return dir;
        } else if(pi < 31) {
            part[pi++] = c;
        }
    }
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

void fs_save_file(fs_entry* file){
    if(!file || file->is_dir || file->lba==0 || !file->content)
        return;
    uint32_t sectors = (file->size + 511) / 512;
    for(uint32_t i=0;i<sectors;i++)
        ata_write_sector(file->lba + i, file->content + i*512);
}

void fs_init(void){

    root_dir.name="/";
    root_dir.is_dir=1;
    root_dir.parent=NULL;
    root_dir.child=NULL;
    root_dir.sibling=NULL;
    root_dir.content=NULL;
    root_dir.lba=0;
    root_dir.size=0;
    root_dir.embedded=0;

    /*
       Read the filesystem header from disk. The header contains the number of
       resource entries, the size of the root table in sectors and the number of
       kernel sectors. Following the header is the table of resource metadata
       describing each file stored on the disk image.
    */
    uint32_t entry_count = 0;
    uint32_t root_sectors = 0;
    uint32_t kernel_sectors = 0;
    unsigned char header[512];
    ata_read_sector(1, header);
    entry_count   = ((uint32_t*)header)[0];
    root_sectors  = ((uint32_t*)header)[1];
    kernel_sectors = ((uint32_t*)header)[2];

    /* Fallback for older images with missing header information */
    if(root_sectors == 0 || kernel_sectors == 0){
        uint32_t kernel_bytes = (uint32_t)&end - 0x1000;
        kernel_sectors = (kernel_bytes + 511) / 512;
        root_sectors = (12 + entry_count*(32+4+4) + 511) / 512;
    }

    /* Read the full root table */
    size_t table_bytes = root_sectors * 512;
    unsigned char* table = mem_alloc(table_bytes);
    if(!table) return;
    for(uint32_t i=0;i<root_sectors;i++)
        ata_read_sector(1 + i, table + i*512);

    unsigned char* p = table + 12;
    for(uint32_t i=0;i<entry_count;i++){
        char name_buf[33];
        for(int j=0;j<32;j++)
            name_buf[j] = p[j];
        name_buf[32] = '\0';
        uint32_t lba = *(uint32_t*)(p + 32);
        uint32_t size = *(uint32_t*)(p + 36);
        p += 32 + 4 + 4;

        fs_entry* dir = &root_dir;
        char part[32];
        size_t pi = 0;
        for(size_t j=0;;j++){
            char c = name_buf[j];
            if(c == '/' || c == 0){
                part[pi] = 0;
                if(c == 0){
                    fs_entry* f = fs_create_file(dir, part);
                    if(f){
                        f->lba = lba;
                        f->size = size;
                        f->embedded = 0;
                        f->content = NULL;
                    }
                    break;
                } else {
                    fs_entry* sub = fs_find_subdir(dir, part);
                    if(!sub)
                        sub = fs_create_dir(dir, part);
                    dir = sub;
                    pi = 0;
                }
            } else if(pi < 31){
                part[pi++] = c;
            }
        }
    }

    /* Ensure the resources directory exists even if no files were
       embedded. This prevents the shell from reporting it missing
       and allows users to add files later. */
    if(!fs_find_subdir(&root_dir, "resources"))
        fs_create_dir(&root_dir, "resources");

}
