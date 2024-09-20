#include "dev/log.h"
#include <exec/module.h>

DEFINE_MODULE("test", load, unload)
PROVIDES("test")

int load() {
	k_info("[test] Module loaded!");
	return 0;
}

int unload() {
	k_info("[test] Module unloaded!");
	return 0;
}
