#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "fcntl.h"
#include "stdio.h"
#include "unistd.h"
#include <wyrm/syscall.h>

int closedir(DIR* dir) {
    int r = close(dir->fd);
    free(dir);
    return r;
}

DIR* opendir(const char* path) {
    int fd = open(path, O_DIR | O_RDONLY);
    if(fd < 0) {
        return NULL;
    }
    DIR* d   = malloc(sizeof(DIR));
    d->fd    = fd;
    d->index = 0;
    return d;
}

struct dirent* readdir(DIR* dir) {
	static struct dirent result;
	uint32_t res = __sys_readdir(dir->fd, dir->index, (uintptr_t) &result);
	dir->index++;

	if(res <= 0) {
		memset(&result, 0, sizeof(struct dirent));
		return NULL;
	}

	return &result;
}

int  readdir_r(DIR *restrict, struct dirent *restrict, struct dirent **restrict) {
    UNIMPL
}

void rewinddir(DIR* dir) {
    dir->index = 0;
}

void seekdir(DIR* d, long index) {
    d->index = index;
}

long telldir(DIR* d) { 
    return d->index;
}

