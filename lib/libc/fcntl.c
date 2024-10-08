#include "wyrm/syscall.h"
#include <fcntl.h>

int open(const char *pathname, int flags, ... /*mode_t mode*/) {
    return __sys_open(pathname, flags);
}

int creat(const char *pathname, mode_t mode) {
    return open(pathname, O_WRONLY|O_CREAT|O_TRUNC, mode);
}

int openat(int dirfd, const char *pathname, int flags, ... /* mode_t mode */ ) {
    // TODO
    return -1;
}
