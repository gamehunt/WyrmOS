#ifndef __K_FS_PATH_H
#define __K_FS_PATH_H 1

#include <types/list.h>

#define P_DIR (1 << 0)

typedef struct {
	list*   data;
	uint8_t flags;
} path;

path* path_create();
path* path_parse(const char* path);
void  path_free(path* p);
char* path_build(path* p);
char* path_filename(path* p);
char* path_folder(path* p);
path* path_join(path* p, const char* part);

#endif
