#ifndef __K_EXEC_H
#define __K_EXEC_H 1

#include <stddef.h>
#include "fs/fs.h"

typedef int(*exec_func)(const char* path, int argc, const char** argv, char** envp);
typedef int(*fmt_checker)(fs_node* file);

typedef struct {
    exec_func   func;
    fmt_checker checker;
} executor;

void k_exec_init();
int  k_exec(const char* exec, int argc, const char** argv, char** envp);
void k_exec_register_executor(executor* ex);

#endif
