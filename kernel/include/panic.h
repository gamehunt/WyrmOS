#ifndef __K_PANIC_H
#define __K_PANIC_H 1

#include "arch.h"

#define MAX_STACKTRACE_DEPTH 16

void panic(regs* registers, const char* message, ...) __attribute__((noreturn));

#endif
