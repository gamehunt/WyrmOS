#ifndef __K_PANIC_H
#define __K_PANIC_H 1

void panic(const char* message, ...) __attribute__((noreturn));

#endif
