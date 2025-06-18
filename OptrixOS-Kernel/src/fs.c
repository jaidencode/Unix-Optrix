#include "fs.h"
#include "mem.h"
#include "resources.h"
#include <stddef.h>

static fs_entry root_dir;

static size_t fs_strlen(const char* s){ size_t l=0; while(s[l]) l++; return l; }

static int streq(const char*a,const char*b){while(*a&&*b){if(*a!=*b)return 0;a++;b++;}return *a==*b;}

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

fs_entry* fs_get_root(void){
    return &root_dir;
}

static void add_child(fs_entry*parent,fs_entry*child){
    child->parent=parent;
    child->sibling=parent->child;
    parent->child=child;
}

fs_entry* fs_create_dir(fs_entry* dir, const char* name){
    fs_entry* e=alloc_entry(name,1);
    if(!e) return NULL;
    add_child(dir,e);
    return e;
}

fs_entry* fs_create_file(fs_entry* dir, const char* name){
    fs_entry* e=alloc_entry(name,0);
    if(!e) return NULL;
    add_child(dir,e);
    return e;
}

fs_entry* fs_find_entry(fs_entry* dir, const char* name){
    for(fs_entry*c=dir->child;c;c=c->sibling)
        if(streq(c->name,name)) return c;
    return NULL;
}

fs_entry* fs_find_subdir(fs_entry* dir, const char* name){
    for(fs_entry*c=dir->child;c;c=c->sibling)
        if(c->is_dir && streq(c->name,name)) return c;
    return NULL;
}

int fs_delete_entry(fs_entry* dir, const char* name){
    fs_entry* prev=NULL; fs_entry* cur=dir->child;
    while(cur){
        if(streq(cur->name,name)){
            if(prev) prev->sibling=cur->sibling; else dir->child=cur->sibling;
            return 1;
        }
        prev=cur; cur=cur->sibling;
    }
    return 0;
}

void fs_write_file(fs_entry* file, const char* text){
    if(!file||file->is_dir) return;
    size_t len=fs_strlen(text)+1;
    file->content=mem_alloc(len);
    if(!file->content) return;
    for(size_t i=0;i<len;i++) file->content[i]=text[i];
}

const char* fs_read_file(fs_entry* file){
    if(!file||file->is_dir||!file->content) return "";
    return file->content;
}

void fs_init(void){
    root_dir.name="/";
    root_dir.is_dir=1;
    root_dir.parent=NULL;
    root_dir.child=NULL;
    root_dir.sibling=NULL;
    root_dir.content=NULL;

    fs_entry* bin=fs_create_dir(&root_dir,"bin");
    (void)bin;
    fs_entry* docs=fs_create_dir(&root_dir,"docs");
    (void)docs;
    fs_entry* desktop=fs_create_dir(&root_dir,"desktop");
    fs_entry* resources=fs_create_dir(&root_dir,"resources");

    fs_entry* readme=fs_create_file(&root_dir,"readme.txt");
    fs_write_file(readme,"Welcome to OptrixOS");

    fs_entry* term=fs_create_file(desktop,"terminal.opt");
    (void)term;

    for(int i=0;i<resource_files_count;i++){
        fs_entry* f=fs_create_file(resources,resource_files[i].name);
        if(f) fs_write_file(f,resource_files[i].data);
    }
}
