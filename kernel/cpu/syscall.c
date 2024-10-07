#include "arch.h"
#include "dev/log.h"
#include "fs/fs.h"
#include "mem/mmap.h"
#include "mem/paging.h"
#include "proc/process.h"
#include <cpu/syscall.h>
#include <stddef.h>

static int sys_open(const char* path, int flags) {
    fs_node* node = k_fs_open(path, flags);
    return k_process_open_file(node);
}

static int sys_write(unsigned int fd, size_t offset, size_t size, void* buffer) {
    if(!validate_ptr(buffer, size)) {
        return -1;
    }
    fs_node* node = k_process_get_file(fd);
    if(!node) {
        return -1;
    }
    return k_fs_write(node, offset, size, buffer);
}

static int sys_read(unsigned int fd, size_t offset, size_t size, void* buffer) {
    if(!validate_ptr(buffer, size)) {
        return -1;
    }
    fs_node* node = k_process_get_file(fd);
    if(!node) {
        return -1;
    }
    return k_fs_read(node, offset, size, buffer);
}

static int sys_fork() {
    return k_process_fork();
}

static int sys_exit(int code) {
    k_process_exit(code);
    return -1;
}

static int sys_kill(pid_t pid, int sig) {
    return k_process_send_signal(pid, sig);
}

static int sys_getpid() {
    return current_core->current_process->pid;
}

static int sys_signal(int sig, uintptr_t handler, uintptr_t* old) {
    if(sig < 0 || sig > NSIG) {
        return -1;
    }
    if(!validate_ptr(old, sizeof(old))) {
        return -1;
    }
    *old = current_core->current_process->signals[sig].handler;
    current_core->current_process->signals[sig].handler = handler;
    return 0;
}

static int sys_sleep(uint64_t seconds, uint64_t subseconds, int relative) {
    if(relative) {
        uint64_t ticks = arch_get_ticks() / arch_get_cpu_speed(); 

        uint64_t seconds_base    = ticks / SUBSECONDS_PER_SECOND;
        uint64_t subseconds_base = ticks % SUBSECONDS_PER_SECOND;

        seconds    += seconds_base;
        subseconds += subseconds_base;
    }
    k_process_sleep(seconds, subseconds);
    k_process_switch(0);
    return 0;
}

static int sys_test() {
    k_debug("Test");
    return 0;
}

static int sys_mmap(uintptr_t* start, size_t size, uint8_t flags, int prot, int fd, off_t offs) {
    if(!validate_ptr(start, sizeof(uintptr_t))) {
        return -1;
    }
    mmap_block* bl = k_mem_mmap(*start, size, flags, prot, fd, offs);
    if(!bl) {
        *start = 0;
        return -1;
    }
    *start = bl->start;
    return 0;
}

static int sys_munmap(uintptr_t start, size_t size) {
    k_mem_munmap(start, size);
    return 0;
}

typedef int (*syscall_handler)(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);
static const syscall_handler __syscall_table[] = {
    [SYS_OPEN]   = (syscall_handler) sys_open,
    [SYS_READ]   = (syscall_handler) sys_read,
    [SYS_WRITE]  = (syscall_handler) sys_write,
    [SYS_FORK]   = (syscall_handler) sys_fork,
    [SYS_EXIT]   = (syscall_handler) sys_exit,
    [SYS_GETPID] = (syscall_handler) sys_getpid,
    [SYS_KILL]   = (syscall_handler) sys_kill,
    [SYS_SIGNAL] = (syscall_handler) sys_signal,
    [SYS_SLEEP]  = (syscall_handler) sys_sleep,
    [SYS_MMAP]   = (syscall_handler) sys_mmap,
    [SYS_MUNMAP] = (syscall_handler) sys_munmap,
    [__SYS_TEST] = (syscall_handler) sys_test
};

static const size_t __syscall_amount = sizeof(__syscall_table) / sizeof(syscall_handler);

int k_invoke_syscall(uint64_t n, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f) {
	if(n >= __syscall_amount || !__syscall_table[n]) {
		k_warn("Invalid syscall: %ld", n);
		return -1;
	}	
    return __syscall_table[n](a, b, c, d, e, f);
}
