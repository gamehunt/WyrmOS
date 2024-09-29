#include "dev/log.h"
#include "proc/process.h"
#include <cpu/syscall.h>
#include <stddef.h>

static int sys_fork() {
    return k_process_fork();
}

typedef int (*syscall_handler)(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);
static const syscall_handler __syscall_table[] = {
    [SYS_FORK] = sys_fork,
};

static const size_t __syscall_amount = sizeof(__syscall_table) / sizeof(syscall_handler);

int k_invoke_syscall(uint64_t n, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f) {
	if(n >= __syscall_amount || !__syscall_table[n]) {
		k_warn("Invalid syscall: %ld", n);
		return -1;
	}	
    return __syscall_table[n](a, b, c, d, e, f);
}


