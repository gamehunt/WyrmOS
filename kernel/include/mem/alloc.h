#ifndef __K_MEM_ALLOC_H
#define __K_MEM_ALLOC_H 1

#include <stddef.h>

void* __attribute__((malloc)) kmalloc(size_t bytes);

// For page-aligned allocations
void* __attribute__((malloc)) vmalloc(size_t bytes);

void  kfree(void* mem);

#endif
