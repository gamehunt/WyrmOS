#ifndef __K_MEM_ALLOC_H
#define __K_MEM_ALLOC_H 1

#include <stddef.h>
#include <mem/paging.h>

void* __attribute__((malloc)) kmalloc(size_t bytes);
void* __attribute__((malloc)) kmalloc_aligned(size_t bytes, size_t aligned);

#define vmalloc(bytes) kmalloc_aligned(bytes, PAGE_SIZE)

void  kfree(void* mem);

#endif
