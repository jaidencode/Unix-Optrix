#include "fs.h"
#include "mem.h"
#include "disk.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_FILES 16

typedef struct {
    char name[16];
    uint32_t lba;
    uint32_t size;
} disk_file;

typedef struct {
    uint32_t count;
    disk_file files[MAX_FILES];
} disk_root;

static disk_root root_table;
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

fs_entry* fs_create_dir(fs_entry* dir, const char* name){ (void)dir;(void)name; return NULL; }
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
fs_entry* fs_find_subdir(fs_entry* dir,const char* name){ (void)dir;(void)name; return NULL; }
int fs_delete_entry(fs_entry* dir,const char* name){ (void)dir;(void)name; return 0; }
void fs_write_file(fs_entry* file,const char* text){ (void)file;(void)text; }

const char* fs_read_file(fs_entry* file){
    if(!file||file->is_dir) return "";
    uint32_t lba = (uint32_t)(uintptr_t)file->content;
    char* buf = mem_alloc(512+1);
    ata_read_sector(lba, buf);
    buf[512]=0;
    return buf;
}

void fs_init(void){
    ata_init();
    ata_read_sector(1, &root_table);
    root_dir.name="/";
    root_dir.is_dir=1;
    root_dir.parent=NULL;
    root_dir.child=NULL;
    root_dir.sibling=NULL;
    root_dir.content=NULL;
    for(uint32_t i=0;i<root_table.count && i<MAX_FILES;i++){
        fs_entry* f=fs_create_file(&root_dir, root_table.files[i].name);
        if(f) f->content=(char*)(uintptr_t)root_table.files[i].lba;
    }
}
