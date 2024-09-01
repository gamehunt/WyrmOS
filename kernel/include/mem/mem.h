#ifndef __K_MEM_H
#define __K_MEM_H 1

#include <stdint.h>

#define CANONICAL_MASK 0xFFFFffffFFFFUL

#define HEAP_START    0xfffff00000000000UL 
#define HEAP_END      0xffffff0000000000UL
#define VIRTUAL_BASE  0xffffffff80000000UL

typedef uint64_t addr;

int k_mem_init();

#endif
