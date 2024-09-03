#ifndef __K_EXEC_MODULE_H
#define __K_EXEC_MODULE_H 1

typedef int(*module_load)(void);
typedef int(*module_unload)(void);

struct module_info {
	const char* name;
	module_load load;
	module_load unload;
};

#define DEFINE_MODULE(modname, load, unload) \
	int load(void); \
	int unload(void); \
	struct module_info __module_info = {.name = modname, .load = load, .unload = unload};

#include <symbols.h>

INTERNAL int k_load_modules();

#endif
