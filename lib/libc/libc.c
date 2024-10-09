#include "string.h"
#include <stddef.h>
#include <stdlib.h>

extern void __init_stdio();

char** environ = NULL;
int __envc = 0;

void __setup_env(char** envp) {
    __envc = 0;
    while(*(envp + __envc)) {
        __envc++;
    }
    environ = malloc(sizeof(char*) * (__envc));
    for(int i = 0; i < __envc; i++) {
        environ[i] = strdup(envp[i]);
    }
    environ[__envc + 1] = NULL; 
}

void __libc_init(int argc, const char** argv, char** envp, int(*main)(int, const char**, char**)) {
    __setup_env(envp);
    __init_stdio();
    exit(main(argc, argv, envp));
}
