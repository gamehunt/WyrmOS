#include <exec/module.h>
#include <stdlib.h>
#include "dev/log.h"
#include "exec/elf.h"
#include "fs/fs.h"

int k_load_modules() {
	k_debug("Loading modules...");

	fs_node* mod_dir = k_fs_open("/modules", FS_O_DIR);
	if(!mod_dir) {
		k_error("Failed to open /modules.");
		return 1;
	}

	dirent dir;
	size_t index = 0;
	while(k_fs_readdir(mod_dir, &dir, index)) {
		path* p = path_parse("/modules");
		path_join(p, dir.name);
		char* module_path = path_build(p);
		fs_node* mod = k_fs_open(module_path, 0);
		if(!mod) {
			k_error("Failed to open module: %s", module_path);
		} else{
			k_info("Loading module: %s", module_path);
			void* buffer = malloc(mod->size);
			k_fs_read(mod, 0, mod->size, buffer);
			k_fs_close(mod);
			struct module_info* module = k_elf_load_module(buffer);
			if(!module) {
				k_error("Failed to load module: invalid exec data");
				continue;
			}
			int r = module->load();
			if(r != 0) {
				k_error("Failed to load module. Error code: %d", r);
			}
		}
		free(module_path);
		path_free(p);
		index++;
	}

	k_debug("Modules loaded.");

	return 0;
}

EXPORT_INTERNAL(k_load_modules)
