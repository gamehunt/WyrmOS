#include "dev/pci.h"
#include "fs/fs.h"
#include <exec/module.h>

DEFINE_MODULE("ahci", load, unload)
PROVIDES("storage")

static fs_node* __try_init_ahci(pci_device* dev) {

}

int load() {
    list* storages = k_dev_pci_find_devices(0x1);
    foreach(node, storages) {
        __try_init_ahci(node->value);
    }
    list_free(storages);
	k_info("Module loaded!");
	return 0;
}

int unload() {
	k_info("Module unloaded!");
	return 0;
}
