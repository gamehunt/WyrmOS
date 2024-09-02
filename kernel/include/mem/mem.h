#ifndef __K_MEM_H
#define __K_MEM_H 1

#include <stdint.h>
#include <symbols.h>

#define CANONICAL_MASK 0xFFFFffffFFFFUL

#define HEAP_START    0xfffff00000000000UL 
#define HEAP_END      0xffffff0000000000UL
#define SYMTABLE      (HEAP_END + 0x1000)
#define VIRTUAL_BASE  0xffffffff80000000UL

typedef uint64_t addr;

INTERNAL int k_mem_init();

#endif
