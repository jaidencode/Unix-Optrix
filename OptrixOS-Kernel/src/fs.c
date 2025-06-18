#include "fs.h"
#include "mem.h"
#include "resources.h"
#include <stddef.h>

static int streq(const char* a, const char* b){
    while(*a && *b){
        if(*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static fs_entry* root_dir = 0;

fs_entry* fs_get_root(void){
    return root_dir;
}

static fs_entry* alloc_entry(const char* name, int is_dir, fs_entry* parent){
    fs_entry* e = mem_alloc(sizeof(fs_entry));
    if(!e) return 0;
    for(int i=0;i<32;i++) e->name[i]=0;
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]=0;
    e->is_dir = is_dir;
    e->parent = parent;
    e->next = NULL;
    e->children = NULL;
    e->content = NULL;
    return e;
}

fs_entry* fs_create_file(fs_entry* dir, const char* name){
    if(!dir || !dir->is_dir) return 0;
    fs_entry* e = alloc_entry(name,0,dir);
    if(!e) return 0;
    e->next = dir->children;
    dir->children = e;
    e->content = mem_alloc(256);
    if(e->content) e->content[0]='\0';
    return e;
}

fs_entry* fs_create_dir(fs_entry* dir, const char* name){
    if(!dir || !dir->is_dir) return 0;
    fs_entry* e = alloc_entry(name,1,dir);
    if(!e) return 0;
    e->next = dir->children;
    dir->children = e;
    return e;
}

int fs_delete_entry(fs_entry* dir, const char* name){
    if(!dir || !name) return 0;
    fs_entry* prev = NULL;
    fs_entry* cur = dir->children;
    while(cur){
        if(streq(cur->name,name)){
            if(prev) prev->next = cur->next;
            else dir->children = cur->next;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

fs_entry* fs_find_entry(fs_entry* dir, const char* name){
    if(!dir) return 0;
    fs_entry* cur = dir->children;
    while(cur){
        if(streq(cur->name,name)) return cur;
        cur = cur->next;
    }
    return 0;
}

void fs_write_file(fs_entry* file, const char* text){
    if(!file || file->is_dir || !file->content) return;
    int k=0;
    while(text && text[k] && k<255){
        file->content[k]=text[k];
        k++;
    }
    file->content[k]='\0';
}

const char* fs_read_file(fs_entry* file){
    if(!file || file->is_dir || !file->content) return "";
    return file->content;
}

int fs_is_empty(fs_entry* dir){
    return (dir && dir->is_dir && dir->children==0);
}

fs_entry* fs_resolve_path(fs_entry* start, const char* path){
    if(!path || !*path) return start;
    fs_entry* cur = (*path=='/') ? fs_get_root() : start;
    int i = (*path=='/') ? 1 : 0;
    char name[32]; int n=0;
    for(;;){
        char c = path[i];
        if(c=='/' || c==0){
            if(n>0){
                name[n]=0;
                if(streq(name,"..")){ if(cur->parent) cur=cur->parent; }
                else if(!streq(name,".")){
                    fs_entry* next = fs_find_entry(cur,name);
                    if(!next) return 0;
                    cur = next;
                }
                n=0;
            }
            if(c==0) break;
        } else {
            if(n<31) name[n++]=c;
        }
        i++;
    }
    return cur;
}

void fs_init(void){
    root_dir = alloc_entry("/",1,NULL);
    fs_entry* bin = fs_create_dir(root_dir,"bin");
    (void)bin;
    fs_entry* docs = fs_create_dir(root_dir,"docs");
    (void)docs;
    fs_entry* desktop = fs_create_dir(root_dir,"desktop");
    fs_entry* readme = fs_create_file(root_dir,"readme.txt");
    if(readme) fs_write_file(readme,"Welcome to OptrixOS");
    if(desktop) fs_create_file(desktop,"terminal.opt");
    for(int i=0;i<resource_files_count;i++){
        fs_entry* f = fs_create_file(root_dir, resource_files[i].name);
        if(f) fs_write_file(f, resource_files[i].data);
    }
}

