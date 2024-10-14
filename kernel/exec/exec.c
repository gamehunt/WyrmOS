#include "dev/log.h"
#include "exec/elf.h"
#include "fcntl.h"
#include "string.h"
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

exec_func __find_func_for_format(fs_node* file) {
    foreach(exect, __exec_list) {
        executor* e = exect->value;
        if(e->checker(file)) {
            return e->func;
        }
    }
    return NULL;
}

static int __elf_checker(fs_node* file) {
    elf_ident ident;
    size_t r = k_fs_read(file, 0, sizeof(elf_ident), (void*) &ident); 
    return r == sizeof(elf_ident) && k_elf_check(&ident) == ELF_CLASS64;
}

static int __shebang_checker(fs_node* file) {
    char s[3];
    size_t r = k_fs_read(file, 0, 3, (void*) &s);
    if(r < 3) {
        return 0;
    }
    return s[0] == '#' && s[1] == '!';
}

static int __exec_shebang(const char* file, int argc, const char** argv, char** envp) {
    fs_node* exec = k_fs_open(file, O_RDONLY);
    if(!exec) {
        k_error("No such file: %s", file);
        return -1;
    }

    char interp[128] = {0};
    k_fs_read(exec, 2, 128, (void*) &interp);
    k_fs_close(exec);

    for(int i = 0; i < 128; i++) {
        if(interp[i] == '\n' || interp[i] == ' ') {
            interp[i] = '\0';
            break;
        }
    }

    k_debug("interp: %s", interp);

    const char* new_argv[argc + 3];
    memset(new_argv, 0, sizeof(char*));

    new_argv[0] = interp;
    new_argv[1] = file;
    for(int i = 0; i < argc; i++) {
        new_argv[2 + i] = argv[i];
    }
    new_argv[argc + 2] = NULL;

    return k_exec(interp, argc + 2, new_argv, envp);
}

static executor __elf_executor = {.func = &k_elf_exec, .checker = &__elf_checker};
static executor __shb_executor = {.func = &__exec_shebang, .checker = &__shebang_checker};

void k_exec_init() {
    k_exec_register_executor(&__elf_executor);
    k_exec_register_executor(&__shb_executor);
}

int k_exec(const char* path, int argc, const char** argv, char** envp) {
    if(!__exec_list) {
        k_error("No executors registered while trying to run %s", path);
        return -1;
    }

	fs_node* exec = k_fs_open(path, O_RDONLY);
	if(!exec) {
        k_error("Exec not found: %s", path);
		return -1;
	}

    exec_func exec_f = __find_func_for_format(exec);
    k_fs_close(exec);
    if(!exec_f) {
        k_error("Unknown executable format: %s", path);
        return -1;
    }

	return exec_f(path, argc, argv, envp);
}
