#ifndef FS_H
#define FS_H

typedef struct fs_entry {
    char* name;
    int is_dir;
    struct fs_entry* parent;
    struct fs_entry* child;
    struct fs_entry* sibling;
    char* content;
} fs_entry;

fs_entry* fs_find_entry(fs_entry* dir, const char* name);
fs_entry* fs_create_file(fs_entry* dir, const char* name);
fs_entry* fs_create_dir(fs_entry* dir, const char* name);
int fs_delete_entry(fs_entry* dir, const char* name);

/* Write text content to a file entry. Text is truncated to 255 chars. */
void fs_write_file(fs_entry* file, const char* text);

/* Read the content of a file entry. Returns empty string for directories */
const char* fs_read_file(fs_entry* file);

void fs_init(void);
fs_entry* fs_get_root(void);
fs_entry* fs_find_subdir(fs_entry* dir, const char* name);

#endif
