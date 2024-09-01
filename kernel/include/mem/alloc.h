#ifndef __K_MEM_ALLOC_H
#define __K_MEM_ALLOC_H 1

#include <stddef.h>

void* kmalloc(size_t bytes);

// For page-aligned allocations
void* vmalloc(size_t bytes);

void  kfree(void* mem);

#endif
