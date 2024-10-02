#include "dev/log.h"
#include <exec/module.h>

DEFINE_MODULE("video", load, unload)
PROVIDES("framebuffer")

int load() {
	k_info("Module loaded!");
	return 0;
}

int unload() {
	k_info("Module unloaded!");
	return 0;
}
