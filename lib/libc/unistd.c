#include <cpu/syscall.h>
#include "wyrm/syscall.h"
#include <unistd.h>

pid_t getpid() {
    return __sys_getpid();
}

int fork() {
    return __sys_fork();
}

int execv(const char* path, char* const argv[]) {
    return 0;
}

int execve(const char* path, char* const argv[], char* const envp[]) {
    return 0;
}

int execvp(const char* path, char* const argv[]) {
    return 0;
}
