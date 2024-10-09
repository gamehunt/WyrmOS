#ifndef __LIBC_DIRENT_H
#define __LIBC_DIRENT_H 1

#define DIRENT_NAME_LENGTH 256

#include <stddef.h>
#include <stdio.h>

struct dirent {
    char name[DIRENT_NAME_LENGTH];
};

#if !defined(__LIBK) && !defined(__KERNEL)

int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
int            readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict);
void           rewinddir(DIR *);
void           seekdir(DIR *, long);
long           telldir(DIR *);

#endif

#endif
