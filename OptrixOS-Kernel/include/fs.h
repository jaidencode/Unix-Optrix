#ifndef FS_H
#define FS_H

typedef struct fs_entry {
    const char* name;
    int is_dir;
    struct fs_entry* parent;
    const struct fs_entry* children;
    int child_count;
} fs_entry;

void fs_init(void);
fs_entry* fs_get_root(void);
fs_entry* fs_find_subdir(fs_entry* dir, const char* name);

#endif
