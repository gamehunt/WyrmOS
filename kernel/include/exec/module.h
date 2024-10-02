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

#ifdef __MODULE
#undef k_print   
#undef k_debug   
#undef k_verbose
#undef k_info
#undef k_warn
#undef k_error
#undef k_crit
#define k_print(...)   k_dev_log(PRINT, __module_info.name, __VA_ARGS__)
#define k_debug(...)   k_dev_log(DEBUG, __module_info.name, __VA_ARGS__)
#define k_verbose(...) k_dev_log(VERBOSE, __module_info.name,  __VA_ARGS__)
#define k_info(...)    k_dev_log(INFO, __module_info.name, __VA_ARGS__)
#define k_warn(...)    k_dev_log(WARNING, __module_info.name, __VA_ARGS__)
#define k_error(...)   k_dev_log(ERROR, __module_info.name, __VA_ARGS__)
#define k_crit(...)    k_dev_log(CRITICAL, __module_info.name, __VA_ARGS__)
#endif

#include <symbols.h>

INTERNAL int k_load_modules();
int k_load_module(const char* path);
int k_module_check_scope(const char* scope);

#endif
