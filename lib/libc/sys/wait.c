#include <sys/wait.h>
#include <wyrm/syscall.h>

pid_t wait(int *status) {
    return waitpid(-1, status, 0);
}

pid_t waitpid(pid_t pid, int* status, int options) {
    return __sys_waitpid(pid, (uintptr_t) status, options);
}
