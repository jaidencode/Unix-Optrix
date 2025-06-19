#ifndef DISKFS_H
#define DISKFS_H
#include <stdint.h>
#include "fs.h"

void diskfs_init(void);
void diskfs_sync(void);
int diskfs_load_file(fs_entry* entry);
void diskfs_write_file(fs_entry* entry, const char* data, uint32_t size);
void diskfs_add_entry(fs_entry* entry);

#define DISKFS_MAX_FILES 64

int diskfs_get_count(void);
const char* diskfs_get_name(int idx);
uint32_t diskfs_get_size(int idx);
uint32_t diskfs_get_start(int idx);

#endif
