#include "dev/log.h"
#include "fs/fs.h"
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

typedef int (*syscall_handler)(uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f);
static const syscall_handler __syscall_table[] = {
    [SYS_OPEN]  = (syscall_handler) sys_open,
    [SYS_READ]  = (syscall_handler) sys_read,
    [SYS_WRITE] = (syscall_handler) sys_write,
    [SYS_FORK]  = (syscall_handler) sys_fork,
    [SYS_EXIT]  = (syscall_handler) sys_exit,
};

static const size_t __syscall_amount = sizeof(__syscall_table) / sizeof(syscall_handler);

int k_invoke_syscall(uint64_t n, uintptr_t a, uintptr_t b, uintptr_t c, uintptr_t d, uintptr_t e, uintptr_t f) {
	if(n >= __syscall_amount || !__syscall_table[n]) {
		k_warn("Invalid syscall: %ld", n);
		return -1;
	}	
    return __syscall_table[n](a, b, c, d, e, f);
}


