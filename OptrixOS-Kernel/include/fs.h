#ifndef FS_H
#define FS_H
#include <stddef.h>

typedef struct fs_entry {
    char name[32];
    int is_dir;
    struct fs_entry* parent;
    struct fs_entry* next;
    struct fs_entry* children;
    char* content;
} fs_entry;

fs_entry* fs_get_root(void);
void fs_init(void);

fs_entry* fs_find_entry(fs_entry* dir, const char* name);
fs_entry* fs_create_file(fs_entry* dir, const char* name);
fs_entry* fs_create_dir(fs_entry* dir, const char* name);
int fs_delete_entry(fs_entry* dir, const char* name);
void fs_write_file(fs_entry* file, const char* text);
const char* fs_read_file(fs_entry* file);
fs_entry* fs_resolve_path(fs_entry* start, const char* path);
int fs_is_empty(fs_entry* dir);

#endif
