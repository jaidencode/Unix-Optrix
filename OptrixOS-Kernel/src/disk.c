#include "disk.h"
#include "mem.h"

/* Create an in-memory disk. */
disk_t* disk_create(uint32_t size, uint32_t sector_size){
    if(sector_size==0) sector_size=512;
    disk_t* d = mem_alloc(sizeof(disk_t));
    if(!d) return NULL;
    d->data = mem_alloc(size);
    if(!d->data) return NULL;
    d->size = size;
    d->sector_size = sector_size;
    return d;
}

void disk_destroy(disk_t* d){
    (void)d; /* memory not freed in this simple kernel */
}

int disk_read(disk_t* d, uint32_t sector, uint8_t* buf, uint32_t count){
    uint32_t off = sector * d->sector_size;
    uint32_t len = count * d->sector_size;
    if(off + len > d->size) return 0;
    for(uint32_t i=0;i<len;i++) buf[i] = d->data[off+i];
    return count;
}

int disk_write(disk_t* d, uint32_t sector, const uint8_t* buf, uint32_t count){
    uint32_t off = sector * d->sector_size;
    uint32_t len = count * d->sector_size;
    if(off + len > d->size) return 0;
    for(uint32_t i=0;i<len;i++) d->data[off+i] = buf[i];
    return count;
}

partition_t* partition_create(disk_t* d, uint32_t start, uint32_t sectors){
    if(start+sectors > d->size / d->sector_size) return NULL;
    partition_t* p = mem_alloc(sizeof(partition_t));
    if(!p) return NULL;
    p->disk=d; p->start=start; p->sectors=sectors; return p;
}

int partition_read(partition_t* p, uint32_t sector, uint8_t* buf, uint32_t count){
    if(sector+count > p->sectors) return 0;
    return disk_read(p->disk, p->start+sector, buf, count);
}

int partition_write(partition_t* p, uint32_t sector, const uint8_t* buf, uint32_t count){
    if(sector+count > p->sectors) return 0;
    return disk_write(p->disk, p->start+sector, buf, count);
}
