#include "iso9660.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// --- CONFIGURE THIS FOR YOUR SYSTEM ---
#define ISO_BASE ((uint8_t*)0x2000000)   // <-- Change to your mapped ISO address!
#define SECTOR_SIZE 2048

typedef struct {
    uint8_t  length;
    uint8_t  ext_attr_length;
    uint32_t extent_start_little;
    uint32_t extent_start_big;
    uint32_t data_length_little;
    uint32_t data_length_big;
    uint8_t  recording_date[7];
    uint8_t  file_flags;
    uint8_t  file_unit_size;
    uint8_t  interleave_gap_size;
    uint16_t volume_sequence_number_little;
    uint16_t volume_sequence_number_big;
    uint8_t  file_id_length;
    char     file_id[1]; // variable
} __attribute__((packed)) iso_dir_record_t;

static uint32_t le32(const void* p) {
    const uint8_t* b = (const uint8_t*)p;
    return b[0] | (b[1]<<8) | (b[2]<<16) | (b[3]<<24);
}

// Compares two filenames case-insensitively, stops at ';' or 0
static int iso_namecmp(const char* a, const char* b) {
    while (*a && *b && *a != ';' && *b != ';') {
        char ca = *a, cb = *b;
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        ++a; ++b;
    }
    if ((*a == ';' || *a == 0) && (*b == ';' || *b == 0)) return 0;
    return (*a) - (*b);
}

void* iso9660_read_file(const char* filename, size_t* filesize) {
    uint8_t* pvd = ISO_BASE + 0x8000; // sector 16
    if (pvd[0] != 1 || memcmp(pvd+1, "CD001", 5) != 0) return NULL;

    // Root dir entry is at byte 156 in the PVD
    iso_dir_record_t* rootrec = (iso_dir_record_t*)(pvd + 156);
    uint32_t root_start = le32(&rootrec->extent_start_little);
    uint32_t root_size  = le32(&rootrec->data_length_little);

    uint8_t* root_dir = ISO_BASE + root_start * SECTOR_SIZE;
    size_t   offset = 0;

    while (offset < root_size) {
        iso_dir_record_t* rec = (iso_dir_record_t*)(root_dir + offset);
        if (rec->length == 0) break;
        if (rec->file_id_length > 0) {
            // Copy and null-terminate the filename
            char namebuf[130] = {0};
            size_t namelen = rec->file_id_length;
            if (namelen > sizeof(namebuf)-1) namelen = sizeof(namebuf)-1;
            memcpy(namebuf, rec->file_id, namelen);
            namebuf[namelen] = 0;

            // Only files, not directories (ignore . and ..)
            if (!(rec->file_flags & 0x02) && namebuf[0] != 0) {
                if (iso_namecmp(namebuf, filename) == 0) {
                    uint32_t file_start = le32(&rec->extent_start_little);
                    uint32_t file_size  = le32(&rec->data_length_little);
                    if (filesize) *filesize = file_size;
                    return (void*)(ISO_BASE + file_start * SECTOR_SIZE);
                }
            }
        }
        offset += rec->length;
    }
    if (filesize) *filesize = 0;
    return NULL;
}

// Public alias keeping header name in sync
void* iso9660_load_file(const char* filename, size_t* filesize) {
    return iso9660_read_file(filename, filesize);
}
