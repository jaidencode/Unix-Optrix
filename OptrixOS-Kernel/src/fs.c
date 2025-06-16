#include "fs.h"
#include <stddef.h>

static int streq(const char* a, const char* b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

#define MAX_ROOT_ENTRIES 20
static fs_entry root_entries[MAX_ROOT_ENTRIES];
static int root_count = 0;

#define MAX_BIN_ENTRIES 5
static fs_entry bin_entries[MAX_BIN_ENTRIES];
static int bin_count = 0;

#define MAX_DOC_ENTRIES 5
static fs_entry docs_entries[MAX_DOC_ENTRIES];
static int docs_count = 0;

/* entries for /desktop */
#define MAX_DESKTOP_ENTRIES 20
static fs_entry desktop_entries[MAX_DESKTOP_ENTRIES];
static int desktop_count = 0;

static fs_entry root_dir = {"/", 1, NULL, root_entries, 0, ""};

#define MAX_EXTRA_DIRS 20
#define MAX_EXTRA_DIR_ENTRIES 20
static fs_entry extra_dir_entries[MAX_EXTRA_DIRS][MAX_EXTRA_DIR_ENTRIES];
static int extra_dir_used = 0;

fs_entry* fs_get_root(void) {
    return &root_dir;
}

void fs_init(void) {
    root_count = 0;
    bin_count = 0;
    docs_count = 0;
    desktop_count = 0;

    /* setup /bin directory */
    fs_entry bin = {"bin", 1, &root_dir, bin_entries, 0, ""};
    root_entries[root_count++] = bin;

    /* setup /docs directory */
    fs_entry docs = {"docs", 1, &root_dir, docs_entries, 0, ""};
    root_entries[root_count++] = docs;

    /* setup /desktop directory */
    fs_entry desktop = {"desktop", 1, &root_dir, desktop_entries, 0, ""};
    root_entries[root_count++] = desktop;
    fs_entry *desktop_ptr = &root_entries[root_count-1];

    /* simple readme */
    fs_entry readme = {"readme.txt", 0, &root_dir, NULL, 0, "Welcome to OptrixOS"};
    root_entries[root_count++] = readme;

    /* /desktop/terminal.opt executable */
    fs_entry term = {"terminal.opt", 0, desktop_ptr, NULL, 0, ""};
    desktop_entries[desktop_count++] = term;
    desktop_ptr->child_count = desktop_count;

    root_dir.child_count = root_count;
}

fs_entry* fs_find_subdir(fs_entry* dir, const char* name) {
    for(int i=0; i<dir->child_count; i++)
        if(dir->children[i].is_dir && name && dir->children[i].name &&
           streq(dir->children[i].name, name))
            return (fs_entry*)&dir->children[i];
    return 0;
}

fs_entry* fs_find_entry(fs_entry* dir, const char* name) {
    for(int i=0; i<dir->child_count; i++)
        if(name && streq(dir->children[i].name, name))
            return &dir->children[i];
    return 0;
}

fs_entry* fs_create_file(fs_entry* dir, const char* name) {
    int limit = (dir == &root_dir) ? MAX_ROOT_ENTRIES : MAX_EXTRA_DIR_ENTRIES;
    if(dir->child_count >= limit) return 0;
    fs_entry* e = &dir->children[dir->child_count++];
    for(int i=0;i<32;i++){e->name[i]=0;}
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]='\0';
    e->is_dir = 0;
    e->parent = dir;
    e->children = NULL;
    e->child_count = 0;
    e->content[0]='\0';
    return e;
}

fs_entry* fs_create_dir(fs_entry* dir, const char* name) {
    int limit = (dir == &root_dir) ? MAX_ROOT_ENTRIES : MAX_EXTRA_DIR_ENTRIES;
    if(dir->child_count >= limit) return 0;
    if(extra_dir_used >= MAX_EXTRA_DIRS) return 0;
    fs_entry* e = &dir->children[dir->child_count++];
    for(int i=0;i<32;i++){e->name[i]=0;}
    int idx=0; while(name[idx] && idx<31){ e->name[idx]=name[idx]; idx++; }
    e->name[idx]='\0';
    e->is_dir = 1;
    e->parent = dir;
    e->children = extra_dir_entries[extra_dir_used];
    e->child_count = 0;
    e->content[0]='\0';
    extra_dir_used++;
    return e;
}

int fs_delete_entry(fs_entry* dir, const char* name) {
    for(int i=0;i<dir->child_count;i++) {
        if(streq(dir->children[i].name,name)) {
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
    int k = 0;
    while(text[k] && k < 255) {
        file->content[k] = text[k];
        k++;
    }
    file->content[k] = '\0';
}

const char* fs_read_file(fs_entry* file) {
    if(!file || file->is_dir) return "";
    return file->content;
}
