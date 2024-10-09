#include <exec/module.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dev/log.h"
#include "exec/elf.h"
#include "fcntl.h"
#include "fs/fs.h"
#include "types/list.h"

static list* __loaded_modules;

static struct module_info* __k_preload_module(const char* path) {
	fs_node* mod = k_fs_open(path, 0);
	if(!mod) {
		k_error("Failed to open module: %s", path);
        return NULL;
	} 

	k_info("Loading module: %s", path);

	void* buffer = malloc(mod->size);
	k_fs_read(mod, 0, mod->size, buffer);
	k_fs_close(mod);

	struct module_info* module = k_elf_load_module(buffer);
	if(!module) {
		k_error("Failed to load module: invalid exec data");
        return NULL;
	}

    k_debug("Preload finished");

    return module;
}

static int __k_load_module(struct module_info* mod) {
    int r = mod->load();
	if(r != 0) {
		k_error("Failed to load module. Error code: %d", r);
	}
    return r;
}

static int __k_check_dependencies(struct module_info* module) {
    const char** dep = module->deps;
    if(!k_module_check_scope(*dep)) {
        k_error("Unresolved dependency %s for %s", *dep, module->name);
        return 0;
    }
    dep++;
    return 1;
}

int k_load_module(const char* path) {
    struct module_info* mod = __k_preload_module(path);
    if(!mod) {
        return -1;
    }
    if(!__k_check_dependencies(mod)) {
        return -2;
    }
    return __k_load_module(mod);
}

static int8_t __scope_comparator(struct module_info* a, const char* scope) {
    if(!a->scopes) {
        return -1;
    }
    const char** scopes = a->scopes;
    while(*scopes) {
        if(!strcmp(*scopes, scope)) {
            return 0;
        }
    }
    return -1;
}

int k_module_check_scope(const char* scope) {
    assert(__loaded_modules != NULL);
    list_node* module = list_find_cmp(__loaded_modules, (void*) scope, (comparator) __scope_comparator);
    return module != NULL;
}

static int __load_recursive(list_node* node, list* cache) {
    struct module_info* mod = node->value;
    list_delete(cache, node);
    if(mod->deps) {
        const char** dependency = mod->deps;
        while(*dependency) {
            const char* scope = *dependency;
            if(!k_module_check_scope(scope)) {
                list_node* module = list_find_cmp(cache, (void*) scope, (comparator) __scope_comparator);
                if(!module) {
                    k_error("Failed to resolve dependency: %s for module %s", scope, mod->name);
                    return -1;
                }
                int r = __load_recursive(module, cache);
                if(r != 0) {
                    k_error("Failed to load dependency: %s for module %s", scope, mod->name);
                    return r;
                }
            }
            dependency++;
        }
    }
    int r = __k_load_module(mod); 
    if (r != 0) {
        k_error("Failed to load module %s: %d", mod->name, r);
    } else {
        list_push_back(__loaded_modules, mod);
    }
    return r;
}

int k_load_modules() {
	k_debug("Loading modules...");

    __loaded_modules = list_create();

	fs_node* mod_dir = k_fs_open("/modules", O_DIR);
	if(!mod_dir) {
		k_error("Failed to open /modules.");
		return 1;
	}

    list* to_load = list_create();

	struct dirent dir;
	size_t index = 0;
	while(k_fs_readdir(mod_dir, &dir, index)) {
		path* p = path_parse("/modules");
		path_join(p, dir.name);
		char* module_path = path_build(p);
        struct module_info* info = __k_preload_module(module_path);
        if(info) {
            list_push_back(to_load, info);
        }
		free(module_path);
		path_free(p);
		index++;
	}

    while(to_load->head) {
        __load_recursive(to_load->head, to_load);
    }

    list_free(to_load);

	k_debug("Modules loaded.");

	return 0;
}
