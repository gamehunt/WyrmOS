#ifndef __LIBC_FNCTL_H
#define __LIBC_FNCTL_H 1

#include <sys/types.h>

#define O_RDONLY (1 << 0) 
#define O_WRONLY (1 << 1)
#define O_RDWR   O_RDONLY | O_WRONLY
#define O_APPEND (1 << 2)
#define O_CREAT  (1 << 3)
#define O_TRUNC  (1 << 4)
#define O_DIR    (1 << 5)

int open(const char *pathname, int flags, ... /*mode_t mode*/);
int creat(const char *pathname, mode_t mode);
int openat(int dirfd, const char *pathname, int flags, ... /* mode_t mode */ );

#endif
