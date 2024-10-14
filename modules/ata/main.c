#include "asm.h"
#include "dev/pci.h"
#include "fs/fs.h"
#include <exec/module.h>
#include <stdio.h>

#define ATA_IO_BASE(p)   ((p) ? 0x1F0 : 0x170)
#define ATA_CTRL_BASE(p) ((p) ? 0x3F6 : 0x376)

#define ATA_DATA(p)    (ATA_IO_BASE(p) + 0)
#define ATA_ERROR(p)   (ATA_IO_BASE(p) + 1)
#define ATA_FEAT(p)    (ATA_IO_BASE(p) + 1)
#define ATA_SEC_CNT(p) (ATA_IO_BASE(p) + 2)
#define ATA_SEC_LO(p)  (ATA_IO_BASE(p) + 3)
#define ATA_SEC_MI(p)  (ATA_IO_BASE(p) + 4)
#define ATA_SEC_HI(p)  (ATA_IO_BASE(p) + 5)
#define ATA_DRIVE(p)   (ATA_IO_BASE(p) + 6)
#define ATA_STATUS(p)  (ATA_IO_BASE(p) + 7)
#define ATA_COMMAND(p) (ATA_IO_BASE(p) + 7)

#define ATA_ALT_STATUS(p) (ATA_CTRL_BASE(p) + 0)
#define ATA_DEV_CTRL(p)   (ATA_CTRL_BASE(p) + 0)
#define ATA_DEV_SELECT(p) (ATA_CTRL_BASE(p) + 1)

DEFINE_MODULE("ata", load, unload)
PROVIDES("storage")

static char        __letter  = 'a';
static pci_device* __ata_pci = NULL;

#define ATA_PRIMARY (1 << 0)
#define ATA_MASTER  (1 << 1)

typedef struct {
    uint8_t type;
} ata_device;

static ata_device __ata_devices[4] = {0};
static uint16_t __ata_ident[256] = {0};

#define is_primary(dev) (dev->type & ATA_PRIMARY)

static void __ata_delay(ata_device* dev) {
    inb(ATA_ALT_STATUS(is_primary(dev)));
    inb(ATA_ALT_STATUS(is_primary(dev)));
    inb(ATA_ALT_STATUS(is_primary(dev)));
    inb(ATA_ALT_STATUS(is_primary(dev)));
}

static void __ata_reset(ata_device* dev) {
    outb(ATA_DEV_CTRL(is_primary(dev)), 0x4);
    __ata_delay(dev);
    outb(ATA_DEV_CTRL(is_primary(dev)), 0x0);
}

static int __ata_send_ident(ata_device* dev) {
    uint8_t p = is_primary(dev);
    uint8_t s = inb(ATA_STATUS(p));
    k_debug("ident: %d (%d)", dev->type, p);
    if(s == 0xFF) {
        k_warn("Floating bus.");
        return 0;
    }
    outb(ATA_DEV_SELECT(p), (dev->type & ATA_MASTER) ? 0xA0 : 0xB0);
    __ata_delay(dev);
    outb(ATA_SEC_CNT(p), 0);
    __ata_delay(dev);
    outb(ATA_SEC_LO(p),  0);
    __ata_delay(dev);
    outb(ATA_SEC_MI(p),  0);
    __ata_delay(dev);
    outb(ATA_SEC_HI(p),  0);
    __ata_delay(dev);
    outb(ATA_COMMAND(p), 0xEC);
    __ata_delay(dev);

    while(1) {
        uint8_t status = inb(ATA_STATUS(p));

        if(status == 0) {
            k_debug("zero status after ident");
            return 0;
        }

        if(status & 0x80) {
            continue;
        }

        if(inb(ATA_SEC_MI(p)) || inb(ATA_SEC_HI(p))) {
            k_debug("broken atapi");
            return 0;
        }

        if(status & 1) {
            k_error("ATA error during identity: %d", inb(ATA_ERROR(p)));
            return 0;
        }

        if(status & 8) {
            break;
        }
    }

    for(int i = 0; i < 256; i++) {
        __ata_ident[i] = inw(ATA_DATA(p));
    }

    return 1;
}

static void __ata_init(ata_device* dev) {
    __ata_reset(dev);

    if(!__ata_send_ident(dev)) {
        return;
    }

    k_debug("LBA28 sectors: %ld", *(uint32_t*)&__ata_ident[60]);
    k_debug("LBA48 sectors: %ld", *(uint64_t*)&__ata_ident[100]);

    uint32_t bar = k_dev_pci_read_dword(__ata_pci->address, PCI_DW_BAR(4));

    if(!(bar & 1)) {
        k_debug("Memory mapped BAR not impl. yet");
        return; 
    } 

    bar &= 0xFFFFFFFC;

    char name[] = {'h', 'd', 'a', ++__letter, '\0'};
    char path[32];

    snprintf(path, 32, "/dev/%s", name);

    fs_node*  ata = k_fs_alloc_fsnode(name);
    // Ops...
    k_fs_mount_node(path, ata);
}

static int __ata_pci_scanner(pci_device* dev) {
    return k_dev_pci_read_byte(dev->address, PCI_B_CLASS) == 0x1 &&
           k_dev_pci_read_byte(dev->address, PCI_B_SUBCLASS) == 0x1;
}

int load() {
    list* storages = k_dev_pci_find_devices(__ata_pci_scanner);
    if(storages->size) {
        __ata_pci = storages->head->value;
    }
    list_free(storages);

    for(int i = 0; i < 4; i++) {
        __ata_devices[i].type = i;
        __ata_init(&__ata_devices[i]);
    }

    k_info("Module loaded!");
    return 0;
}

int unload() {
    k_info("Module unloaded!");
    return 0;
}
