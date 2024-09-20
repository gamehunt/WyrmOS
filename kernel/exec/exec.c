#include "dev/log.h"
#include "exec/elf.h"
#include "types/list.h"
#include <exec/exec.h>
#include <fs/fs.h>
#include <stdlib.h>

static list* __exec_list = NULL;

void k_exec_register_executor(executor* ex) {
    if(!__exec_list) {
        __exec_list = list_create();
    }
    list_push_back(__exec_list, ex);
}

exec_func __find_func_for_format(void* file, size_t size) {
    foreach(exect, __exec_list) {
        executor* e = exect->value;
        if(e->checker(file, size)) {
            return e->func;
        }
    }
    return NULL;
}

static int __elf_checker(void* file, size_t size) {
    return k_elf_check(file) == ELF_CLASS64;
}

static executor __elf_executor = {.func = &k_elf_exec, .checker = &__elf_checker};

void k_exec_init() {
    k_exec_register_executor(&__elf_executor);
}

int k_exec(const char* path, int argc, const char** argv, const char** envp) {
    if(!__exec_list) {
        k_error("No executors registered while trying to run %s", path);
        return -1;
    }

	fs_node* exec = k_fs_open(path, 0);
	if(!exec) {
        k_error("Exec not found: %s", path);
		return -1;
	}

    void* data = malloc(exec->size);
    size_t r = k_fs_read(exec, 0, exec->size, data);
    if(r < exec->size) {
        k_error("Failed to read: %s", path);
        free(data);
        return -1;
    }

    exec_func exec_f = __find_func_for_format(data, exec->size);
    if(!exec_f) {
        k_error("Unknown executable format: %s", path);
        free(data);
        return -1;
    }

	return exec_f(data, argc, argv, envp);
}
