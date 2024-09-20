#ifndef __K_EXEC_MODULE_H
#define __K_EXEC_MODULE_H 1

typedef int(*module_load)(void);
typedef int(*module_unload)(void);

struct module_info {
	const char*  name;
	module_load  load;
	module_load  unload;
    const char** scopes;
    const char** deps;
};

#define DEFINE_MODULE(modname, load, unload) \
	int load(void); \
	int unload(void); \
    extern const char* __module_info_scopes[] __attribute__((weak));      \
    extern const char* __module_info_dependencies[] __attribute__((weak)); \
	struct module_info __module_info = {.name = modname, .load = load, .unload = unload, \
        .scopes = __module_info_scopes, .deps = __module_info_dependencies};

#define PROVIDES(scopes) \
    const char* __module_info_scopes[] = {scopes, "\0"};

#define DEPENDS_ON(dependencies) \
    const char* __module_info_dependencies[] = {dependencies, "\0"};

#include <symbols.h>

INTERNAL int k_load_modules();
int k_load_module(const char* path);
int k_module_check_scope(const char* scope);

#endif
