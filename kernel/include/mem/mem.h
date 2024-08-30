#ifndef __K_MEM_H
#define __K_MEM_H 1

#include <stdint.h>

#define VIRTUAL_BASE  0xffffffff80000000

typedef uint64_t addr;

int k_mem_init();

#endif
