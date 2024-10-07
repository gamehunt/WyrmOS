#ifndef __LIBC_WYRM_SYSCALL_H
#define __LIBC_WYRM_SYSCALL_H 1

#include <cpu/syscall.h>
#include <stdint.h>

extern int __invoke_syscall(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f, int number);

#define INVOKE_SYSCALL0(n) __invoke_syscall(0, 0, 0, 0, 0, 0, n)
#define INVOKE_SYSCALL1(n, a) __invoke_syscall(a, 0, 0, 0, 0, 0, n)
#define INVOKE_SYSCALL2(n, a, b) __invoke_syscall(a, b, 0, 0, 0, 0, n)
#define INVOKE_SYSCALL3(n, a, b, c) __invoke_syscall(a, b, c, 0, 0, 0, n)
#define INVOKE_SYSCALL4(n, a, b, c, d) __invoke_syscall(a, b, c, d, 0, 0, n)
#define INVOKE_SYSCALL5(n, a, b, c, d, e) __invoke_syscall(a, b, c, d, e, 0, n)
#define INVOKE_SYSCALL6(n, a, b, c, d, e, f) __invoke_syscall(a, b, c, d, e, f, n)

#define __sys_fork()                            INVOKE_SYSCALL0(SYS_FORK)
#define __sys_exit(code)                        INVOKE_SYSCALL1(SYS_EXIT, code)
#define __sys_getpid()                          INVOKE_SYSCALL0(SYS_GETPID)
#define __sys_kill(pid, s)                      INVOKE_SYSCALL2(SYS_KILL, pid, s)
#define __sys_signal(s, handl, old)             INVOKE_SYSCALL3(SYS_SIGNAL, s, (uintptr_t) handl, (uintptr_t) old)
#define __sys_sleep(seconds, microseconds, rel) INVOKE_SYSCALL3(SYS_SLEEP, seconds, microseconds, rel)
#define __sys_mmap(st, sz, fl, pr, fd, of)      INVOKE_SYSCALL6(SYS_MMAP, st, sz, fl, pr, fd, of)
#define __sys_munmap(st, sz)                    INVOKE_SYSCALL2(SYS_MUNMAP, st, sz)
#define __sys_read(fd, offs, size, buffer)      INVOKE_SYSCALL4(SYS_READ, fd, offs, size, buffer)
#define __sys_write(fd, offs, size, buffer)     INVOKE_SYSCALL4(SYS_WRITE, fd, offs, size, buffer)
#define __sys_close(fd)                         INVOKE_SYSCALL1(SYS_CLOSE, fd)
#define __sys_open(path, flags)                 INVOKE_SYSCALL2(SYS_OPEN, (uintptr_t) path, flags)

#endif
