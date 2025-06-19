#include "fs.h"
#include "mem.h"
#include "root_files.h"
#include "disk.h"
#include "build_config.h"
#include <stddef.h>
#include <stdint.h>

static fs_entry root_dir;
static unsigned int next_free_sector = FS_START_SECTOR;

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
    e->disk_offset=0;
    e->disk_size=0;
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

    /* Store the content on disk as well */
    unsigned int sectors = (len + 511) / 512;
    file->disk_offset = next_free_sector;
    file->disk_size = len;
    uint8_t sec[512];
    for(unsigned int s=0;s<sectors;s++){
        for(int i=0;i<512;i++){
            unsigned int idx=s*512+i;
            sec[i] = idx<len ? (uint8_t)text[idx] : 0;
        }
        disk_write_sector(next_free_sector+s, sec);
    }
    next_free_sector += sectors;
}

const char* fs_read_file(fs_entry* file){
    if(!file || file->is_dir)
        return "";
    if(!file->content && file->disk_size){
        char* buf = mem_alloc(file->disk_size + 1);
        if(!buf) return "";
        unsigned int sectors = (file->disk_size + 511)/512;
        uint8_t sec[512];
        for(unsigned int s=0;s<sectors;s++){
            disk_read_sector(file->disk_offset + s, sec);
            for(int i=0;i<512;i++){
                unsigned int idx=s*512+i;
                if(idx<file->disk_size)
                    buf[idx] = sec[i];
            }
        }
        buf[file->disk_size] = '\0';
        file->content = buf;
    }
    if(!file->content)
        return "";
    return file->content;
}

void fs_init(void){
    root_dir.name="/";
    root_dir.is_dir=1;
    root_dir.parent=NULL;
    root_dir.child=NULL;
    root_dir.sibling=NULL;
    root_dir.content=NULL;
    root_dir.disk_offset=0;
    root_dir.disk_size=0;
    for(int i=0;i<root_files_count;i++){
        const char* path = root_files[i].path;
        fs_entry* dir = &root_dir;
        char part[32]; int pi=0;
        for(int j=0;;j++){
            char c=path[j];
            if(c=='/' || c==0){
                part[pi]=0;
                if(c==0){
                    fs_entry* f=fs_create_file(dir, part);
                    if(f) fs_write_file(f, root_files[i].data);
                    break;
                } else {
                    fs_entry* d=fs_find_subdir(dir, part);
                    if(!d) d=fs_create_dir(dir, part);
                    if(!d) break;
                    dir=d;
                    pi=0;
                }
            } else if(pi<31){
                part[pi++]=c;
            }
        }
    }
}
