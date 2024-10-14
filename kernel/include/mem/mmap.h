#ifndef __K_MEM_MMAP
#define __K_MEM_MMAP 1

#include <stdint.h>
#include <stddef.h>

#include <fs/fs.h>
#include <sys/mman.h>


typedef struct {
    uintptr_t start;
    size_t    size;
    uint8_t   flags;
    int       fd;
    off_t     offset;
} mmap_block;

mmap_block* k_mem_mmap(uintptr_t start, size_t size, uint8_t flags, int prot, int fd, off_t offset);
void        k_mem_munmap(uintptr_t start, size_t size);
void        k_mem_unmap_block(mmap_block* bl);

#endif
