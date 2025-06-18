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

/* Write text content to a file entry. Text is truncated to 255 chars. */
void fs_write_file(fs_entry* file, const char* text);

/* Read the content of a file entry. Returns empty string for directories */
const char* fs_read_file(fs_entry* file);

void fs_init(void);
fs_entry* fs_get_root(void);
fs_entry* fs_find_subdir(fs_entry* dir, const char* name);

/* Resolve a path starting from the provided directory. Paths beginning with
 * '/' are treated as absolute. Supports '.', '..' and nested directories. */
fs_entry* fs_resolve_path(fs_entry* start, const char* path);

/* Construct the full path for an entry. */
void fs_get_path(fs_entry* entry, char* out, int max);

void fs_load_initrd(const unsigned char* data, unsigned int size);

#endif
