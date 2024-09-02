#ifndef __K_EXEC_INITRD_H
#define __K_EXEC_INITRD_H 1

#include <stddef.h>

void k_exec_initrd_init();
int k_exec_initrd_load(void* address, size_t size);

#endif
