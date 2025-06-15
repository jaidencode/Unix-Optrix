#ifndef ISO9660_H
#define ISO9660_H

#include <stddef.h>
#include <stdint.h>

/**
 * iso9660_load_file
 * Load a file from the ISO9660 image. The path may include directories
 * (e.g. "DIR/FILE.TXT"). Names are compared case-insensitively and may use the
 * typical 8.3 format.
 *
 * @param path     Path of the file to load, using '/' as separator.
 * @param filesize Output: length of file if found, else 0.
 * @return         Pointer to file data in memory if found, NULL if not.
 */
void* iso9660_load_file(const char* path, size_t* filesize);

#endif // ISO9660_H
