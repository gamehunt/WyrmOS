#include "dev/pci.h"
#include <exec/module.h>
#include <stdlib.h>

DEFINE_MODULE("ata", load, unload)
PROVIDES("storage")


int load() {
    list* storages = k_dev_pci_find_devices(0x1);
    foreach(_dev, storages) {
        pci_device* dev = _dev->value;
        k_debug("Storage: %d %d %d", DEV_ADDR(dev));
    }
    void* test = malloc(123);
	k_info("Module loaded!");
	return 0;
}

int unload() {
	k_info("Module unloaded!");
	return 0;
}
