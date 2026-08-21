#ifndef EXOMFS_H
#define EXOMFS_H

#include <stdint.h>

struct exomfs_header {
    uint32_t magic;
    uint32_t file_count;
} __attribute__((packed));

struct exomfs_entry {
    char name[24];
    uint32_t offset;
    uint32_t size;
} __attribute__((packed));

uint8_t exomfs_init();
int exomfs_find(const char *name);
uint32_t exomfs_read(int index, void *buf);

#endif