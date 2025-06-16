#ifndef FS_H
#define FS_H

typedef struct fs_entry {
    char name[32];
    int is_dir;
    struct fs_entry* parent;
    struct fs_entry* children;
    int child_count;
    char content[256];
} fs_entry;

fs_entry* fs_find_entry(fs_entry* dir, const char* name);
fs_entry* fs_create_file(fs_entry* dir, const char* name);
fs_entry* fs_create_dir(fs_entry* dir, const char* name);
int fs_delete_entry(fs_entry* dir, const char* name);

void fs_init(void);
fs_entry* fs_get_root(void);
fs_entry* fs_find_subdir(fs_entry* dir, const char* name);

#endif
