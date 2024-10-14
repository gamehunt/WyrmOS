#ifndef __SYS_WAIT_H
#define __SYS_WAIT_H 1

#include "sys/types.h"

pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);

#endif
