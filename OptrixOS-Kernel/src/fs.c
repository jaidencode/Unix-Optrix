#include "fs.h"
#include <stddef.h>

static int streq(const char* a, const char* b) {
    while(*a && *b) {
        if(*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

static fs_entry bin_entries[] = {
    {"echo", 0, NULL, NULL, 0},
    {"ping", 0, NULL, NULL, 0},
};

static fs_entry docs_entries[] = {
    {"guide.txt", 0, NULL, NULL, 0},
    {"info.txt", 0, NULL, NULL, 0},
};

static fs_entry root_entries[] = {
    {"bin", 1, NULL, bin_entries, 2},
    {"docs", 1, NULL, docs_entries, 2},
    {"readme.txt", 0, NULL, NULL, 0},
};

static fs_entry root_dir = {"/", 1, NULL, root_entries, 3};

fs_entry* fs_get_root(void) {
    return &root_dir;
}

void fs_init(void) {
    for(int i=0; i<root_dir.child_count; i++)
        root_entries[i].parent = &root_dir;
    for(int i=0; i<2; i++)
        bin_entries[i].parent = &root_entries[0];
    for(int i=0; i<2; i++)
        docs_entries[i].parent = &root_entries[1];
}

fs_entry* fs_find_subdir(fs_entry* dir, const char* name) {
    for(int i=0; i<dir->child_count; i++)
        if(dir->children[i].is_dir && name && dir->children[i].name &&
           streq(dir->children[i].name, name))
            return (fs_entry*)&dir->children[i];
    return 0;
}
