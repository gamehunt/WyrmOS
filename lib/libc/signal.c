#include "signal.h"
#include "unistd.h"
#include "wyrm/syscall.h"
#include <stddef.h>

int raise(int signal) {
    return kill(getpid(), signal);
}

int kill(pid_t pid, int sig) {
    return __sys_kill(pid, sig);
}

void (*signal (int signal, void (*sigfunc) (int func)))(int) {
    void* old = NULL;
    __sys_signal(signal, sigfunc, old);
    return old;
}
