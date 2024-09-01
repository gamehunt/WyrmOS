#ifndef __K_FS_PATH_H
#define __K_FS_PATH_H 1

#include <types/list.h>

typedef list path;

path* path_parse(const char* path);
void  path_free(path* p);
char* path_build(path* p);
char* path_filename(path* p);
char* path_folder(path* p);
path* path_join(path* p, const char* part);

#define path_create() list_create()

#endif
