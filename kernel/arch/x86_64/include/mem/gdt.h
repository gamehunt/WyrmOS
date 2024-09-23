#ifndef __K_GDT_H 
#define __K_GDT_H 1

#include "symbols.h"

INTERNAL int  k_mem_gdt_init();
INTERNAL void k_mem_flush_gdt(int core);

void k_mem_set_kernel_stack(uintptr_t stack);

#endif
