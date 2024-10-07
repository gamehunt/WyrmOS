#ifndef __K_MEM_H
#define __K_MEM_H 1

#include <stdint.h>
#include <util.h>

#define CANONICAL_MASK 0xFFFFffffFFFFUL

#define MMAP_START     0x0000010000000000UL
#define MMAP_END       0x0000020000000000UL
#define USR_HEAP_START 0x0000030000000000UL
#define HEAP_START     0xfffff00000000000UL 
#define HEAP_END       0xffffff0000000000UL
#define MMIO_START     0xffffff1fc0000000UL
#define MMIO_END       0xffffff2000000000UL
#define SYMTABLE       0xffffff3000000000UL
#define VIRTUAL_BASE   0xffffffff80000000UL
#define KERNEL_LOWEST_ADDRESS HEAP_START
#define USER_STACK_SIZE   KB(64)
#define KERNEL_STACK_SIZE KB(32)
#define PAGE_SIZE 0x1000

typedef uint64_t addr;


#endif
