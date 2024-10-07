#include <sys/mman.h>
#include <wyrm/syscall.h>

void* mmap(void* start, size_t length, int prot , int flags, int fd, off_t offset) {
    uintptr_t _start = (uintptr_t) start; 
    int r = __sys_mmap((uintptr_t) &_start, length, prot, flags, fd, offset);
    if(r) {
        return MAP_FAILED;
    }
    return (void*) _start;
}

int munmap(void *start, size_t length) {
    return __sys_munmap((uintptr_t) start, length);
}
