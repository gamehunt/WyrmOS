#ifndef __SYS_MMAN_H
#define __SYS_MMAN_H 1

#include <stddef.h>
#include <sys/types.h>

#define PROT_READ  (1 << 0)
#define PROT_WRITE (1 << 1)
#define PROT_EXEC  (1 << 3)
#define PROT_NONE  0

#define MAP_FIXED     (1 << 0)
#define MAP_SHARED    (1 << 1)
#define MAP_PRIVATE   (1 << 2)
#define MAP_ANONYMOUS (1 << 3)

#define MAP_FAILED ((void*) -1)

#if !defined(__LIBK) && !defined(__KERNEL)
void* mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset);
int   munmap(void *start, size_t length);
#endif

#endif
