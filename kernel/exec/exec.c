#include "exec/elf.h"
#include <exec/exec.h>
#include <fs/fs.h>

int k_exec(const char* path, int argc, const char** argv, const char** envp) {
	fs_node* exec = k_fs_open(path, 0);
	if(!exec) {
		return -1;
	}

	return k_elf_exec(exec, argc, argv, envp);
}
