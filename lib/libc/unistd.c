#include <unistd.h>

pid_t getpid() {
    return 0;
}

pid_t fork() {
    return 0;
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
