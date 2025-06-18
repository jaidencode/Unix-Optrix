#include "fs.h"
#include "mem.h"
#include <stddef.h>
#include <stdint.h>

static int streq(const char *a, const char *b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

typedef struct __attribute__((packed)) {
    char magic[4];
    uint32_t count;
} fs_header_t;

typedef struct __attribute__((packed)) {
    uint8_t is_dir;
    uint8_t reserved[3];
    uint32_t parent;
    char name[32];
    uint32_t size;
    uint32_t offset;
} fs_entry_disk_t;

static fs_entry *fs_entries = NULL;
static int fs_entry_count = 0;
static fs_entry *root_dir = NULL;

fs_entry* fs_get_root(void) {
    return root_dir;
}

static void add_child(fs_entry *parent, fs_entry *child) {
    fs_entry **newarr = mem_alloc(sizeof(fs_entry*)*(parent->child_count+1));
    for(int i=0;i<parent->child_count;i++)
        newarr[i] = parent->children[i];
    newarr[parent->child_count] = child;
    parent->children = newarr;
    parent->child_count++;
}

void fs_init(unsigned int addr, unsigned int size) {
    fs_header_t *hdr = (fs_header_t*)addr;
    if(hdr->magic[0] != 'O' || hdr->magic[1] != 'P' || hdr->magic[2] != 'F' || hdr->magic[3] != 'S')
        return;
    fs_entry_count = hdr->count;
    fs_entries = mem_alloc(sizeof(fs_entry)*fs_entry_count);
    fs_entry_disk_t *disk = (fs_entry_disk_t*)(addr + sizeof(fs_header_t));
    const char *data_base = (const char*)(disk + fs_entry_count);
    for(int i=0;i<fs_entry_count;i++) {
        fs_entry *e = &fs_entries[i];
        fs_entry_disk_t *de = &disk[i];
        for(int k=0;k<32;k++) e->name[k] = de->name[k];
        e->name[31] = '\0';
        e->is_dir = de->is_dir;
        e->parent = NULL;
        e->children = NULL;
        e->child_count = 0;
        if(e->is_dir) {
            e->content = "";
            e->size = 0;
        } else {
            e->content = data_base + de->offset;
            e->size = de->size;
        }
    }
    for(int i=0;i<fs_entry_count;i++) {
        uint32_t p = disk[i].parent;
        if(p < (uint32_t)fs_entry_count)
            fs_entries[i].parent = &fs_entries[p];
    }
    for(int i=0;i<fs_entry_count;i++)
        if(fs_entries[i].parent)
            fs_entries[i].parent->child_count++;
    for(int i=0;i<fs_entry_count;i++) {
        if(fs_entries[i].child_count>0) {
            fs_entries[i].children = mem_alloc(sizeof(fs_entry*)*fs_entries[i].child_count);
            fs_entries[i].child_count = 0;
        }
    }
    for(int i=0;i<fs_entry_count;i++) {
        fs_entry *p = fs_entries[i].parent;
        if(p)
            p->children[p->child_count++] = &fs_entries[i];
    }
    root_dir = &fs_entries[0];
}

fs_entry* fs_find_subdir(fs_entry* dir, const char* name) {
    for(int i=0;i<dir->child_count;i++)
        if(dir->children[i]->is_dir && streq(dir->children[i]->name, name))
            return dir->children[i];
    return NULL;
}

fs_entry* fs_find_entry(fs_entry* dir, const char* name) {
    for(int i=0;i<dir->child_count;i++)
        if(streq(dir->children[i]->name, name))
            return dir->children[i];
    return NULL;
}

fs_entry* fs_create_file(fs_entry* dir, const char* name) {
    fs_entry *e = mem_alloc(sizeof(fs_entry));
    if(!e) return NULL;
    for(int i=0;i<32;i++){e->name[i]=0;}
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]='\0';
    e->is_dir = 0;
    e->parent = dir;
    e->children = NULL;
    e->child_count = 0;
    e->size = 0;
    e->content = mem_alloc(256);
    if(!e->content) return NULL;
    ((char*)e->content)[0]='\0';
    add_child(dir, e);
    return e;
}

fs_entry* fs_create_dir(fs_entry* dir, const char* name) {
    fs_entry *e = mem_alloc(sizeof(fs_entry));
    if(!e) return NULL;
    for(int i=0;i<32;i++){e->name[i]=0;}
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]='\0';
    e->is_dir = 1;
    e->parent = dir;
    e->children = NULL;
    e->child_count = 0;
    e->content = "";
    e->size = 0;
    add_child(dir, e);
    return e;
}

int fs_delete_entry(fs_entry* dir, const char* name) {
    for(int i=0;i<dir->child_count;i++) {
        if(streq(dir->children[i]->name,name)) {
            for(int j=i;j<dir->child_count-1;j++)
                dir->children[j]=dir->children[j+1];
            dir->child_count--;
            return 1;
        }
    }
    return 0;
}

void fs_write_file(fs_entry* file, const char* text) {
    if(!file || file->is_dir) return;
    char *buf = (char*)file->content;
    int k=0;
    while(text[k] && k<255){ buf[k]=text[k]; k++; }
    buf[k]='\0';
    file->size = k;
}

const char* fs_read_file(fs_entry* file) {
    if(!file || file->is_dir) return "";
    return file->content;
}
