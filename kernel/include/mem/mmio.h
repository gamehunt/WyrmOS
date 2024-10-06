#ifndef __K_MEM_MMIO_H 
#define __K_MEM_MMIO_H 1

#include <stddef.h>
#include <stdint.h>

void* k_mem_iomap(uintptr_t phys, size_t size);
void  k_mem_iounmap(void* mem);

void* k_mem_alloc_dma(size_t size, uintptr_t* phys);

#endif
