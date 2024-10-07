#ifndef __K_MEM_MMAP
#define __K_MEM_MMAP 1

#include <stdint.h>
#include <stddef.h>

#include <fs/fs.h>

#define PROT_READ  (1 << 0)
#define PROT_WRITE (1 << 1)
#define PROT_EXEC  (1 << 3)
#define PROT_NONE  0

#define MAP_FIXED     (1 << 0)
#define MAP_SHARED    (1 << 1)
#define MAP_PRIVATE   (1 << 2)
#define MAP_ANONYMOUS (1 << 3)

typedef struct {
    uintptr_t start;
    size_t    size;
    uint8_t   flags;
    int       fd;
} mmap_block;

mmap_block* k_mem_mmap(uintptr_t start, size_t size, uint8_t flags, int prot, int fd);
void        k_mem_unmap(uintptr_t start, size_t size);

#endif
