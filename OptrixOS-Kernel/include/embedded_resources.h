#ifndef EMBEDDED_RESOURCES_H
#define EMBEDDED_RESOURCES_H
#include <stdint.h>

typedef struct {
    const char* name;
    const unsigned char* data;
    uint32_t size;
} embedded_resource;

#define EMBEDDED_RESOURCE_COUNT 0
extern const embedded_resource embedded_resources[EMBEDDED_RESOURCE_COUNT];

#endif
