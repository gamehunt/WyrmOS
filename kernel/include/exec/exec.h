#ifndef __K_EXEC_H
#define __K_EXEC_H 1

#include <stddef.h>

typedef int(*exec_func)(void* data, int argc, const char** argv, const char** envp);
typedef int(*fmt_checker)(void* data, size_t size);

typedef struct {
    exec_func   func;
    fmt_checker checker;
} executor;

void k_exec_init();
int  k_exec(const char* exec, int argc, const char** argv, const char** envp);
void k_exec_register_executor(executor* ex);

#endif
