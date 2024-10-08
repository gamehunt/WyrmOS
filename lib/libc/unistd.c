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

unsigned int sleep(unsigned int seconds) {
    return __sys_sleep(seconds, 0, 1);
}

int usleep(unsigned long usec) {
    return __sys_sleep(usec / 1000000, usec % 1000000, 1);
}

ssize_t read(int fd, void* buf, size_t count) {
    return __sys_read(fd, count, (uintptr_t) buf);
}

ssize_t write(int fd, const void* buf, size_t count) {
    return __sys_write(fd, count, (uintptr_t) buf);
}

int close(int fd) {
    return __sys_close(fd);
}
