#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#define MULTIBOOT_MAGIC 0x1BADB002
#define MULTIBOOT_FLAG  0

struct multiboot_header {
    unsigned long magic;
    unsigned long flags;
    unsigned long checksum;
};

#endif /* MULTIBOOT_H */
