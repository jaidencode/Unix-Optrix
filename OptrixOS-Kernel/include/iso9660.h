#ifndef ISO9660_H
#define ISO9660_H

#include <stddef.h>
#include <stdint.h>

/**
 * iso9660_load_file
 * Searchs for a file by name in the ISO9660 root directory (uppercase, no path).
 * 
 * @param filename    The filename to search for (e.g., "POINTING_HAND.CUR").
 * @param filesize    Output: length of file if found, else 0.
 * @return            Pointer to file data in memory if found, NULL if not.
 */
void* iso9660_load_file(const char* filename, size_t* filesize);

#endif // ISO9660_H
