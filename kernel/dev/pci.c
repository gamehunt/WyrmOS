#include "asm.h"
#include "dev/acpi.h"
#include "dev/log.h"
#include "mem/mmio.h"
#include "assert.h"
#include "types/list.h"
#include <dev/pci.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    uint8_t    access_type;
    acpi_mcfg* mcfg;
} pci_configuration;

static pci_configuration __pci_conf = {0};
static list* __devices = NULL;


uint32_t k_dev_pci_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset) {
    assert(offset < 4096);
    if(__pci_conf.access_type == 0) {
        uint32_t addr = ((uint32_t) bus << 16) | 
            ((uint32_t) device << 11) | 
            ((uint32_t) function << 8) | (offset & 0xFC)
            | ((uint32_t)0x80000000);
        outl(0xCF8, addr);
        return inl(0xCFC);
    } else if (__pci_conf.access_type == 2){
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(__pci_conf.mcfg, conf_spaces); i++) {
            if(bus >= __pci_conf.mcfg->conf_spaces[i].start_bus && 
                    bus <= __pci_conf.mcfg->conf_spaces[i].end_bus) {
                void* mem = k_mem_iomap(__pci_conf.mcfg->conf_spaces[i].address + 
                        (((uintptr_t)bus << 20) | ((uintptr_t)device << 15) | ((uintptr_t)function << 12)), 4096);
                return *(uint32_t*)(mem + (offset & 0xFC));
            }
        }
    }
    return 0;
}

uint16_t k_dev_pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset) {
    uint32_t word = offset & 2; 
    return (k_dev_pci_read_dword(bus, device, function, offset) >> (word * 8)) & 0xFFFF;
}

uint8_t  k_dev_pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset) {
    uint32_t byte = offset & 1; 
    return (k_dev_pci_read_word(bus, device, function, offset) >> (byte * 8)) & 0xFF;
}

pci_device* k_dev_pci_create_device(uint8_t bus, uint8_t device, uint8_t function) {
    pci_device* dev = malloc(sizeof(pci_device));
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    return dev;
}

static void __k_dev_probe_bus(int bus);
static void __k_dev_probe_device(int bus, uint8_t device, uint8_t function) {
    uint16_t vendor = k_dev_pci_read_word(bus, device, function, PCI_W_VENDOR);
    if(vendor == 0xFFFF) {
        return;
    }
    uint16_t dev = k_dev_pci_read_word(bus, device, function, PCI_W_DEVICE);
    k_debug("%d:%d:%d: 0x%.4x 0x%.4x", bus, device, function, vendor, dev);
    list_push_back(__devices, k_dev_pci_create_device(bus, device, function));
    if(function == 0) {
        uint8_t header_type = k_dev_pci_read_byte(bus, device, function, PCI_B_HTYPE);
        if(header_type & 0x80) {
            for(int i = 1; i < 8; i++) {
                __k_dev_probe_device(bus, device, i);
            }
        }
    }
    uint8_t class    = k_dev_pci_read_byte(bus, device, function, PCI_B_CLASS);
    uint8_t subclass = k_dev_pci_read_byte(bus, device, function, PCI_B_SUBCLASS);
    if(class == 0x6 && subclass == 0x4) {
        k_debug("Bridge detected: 0x%.4x 0x%.4x", vendor, dev);
        __k_dev_probe_bus(k_dev_pci_read_byte(bus, device, function, 0x19));
    }
}

static void __k_dev_probe_bus(int bus) {
    for(int device = 0; device < 32; device++) {
        __k_dev_probe_device(bus, device, 0);
    }
}

list* k_dev_pci_get_devices() {
    return __devices;
}

list*  k_dev_pci_find_devices(uint8_t class) {
    list* r = list_create();
    foreach(node, __devices) {
        uint8_t class = k_dev_pci_read_byte(DEV_ADDR(((pci_device*)node->value)), PCI_B_CLASS);
        if(class == 0x1) {
            list_push_back(r, node->value);
        }
    }
    return r;
}

void k_dev_pci_init() {
    __devices = list_create();
    acpi_mcfg* mcfg = (acpi_mcfg*) k_dev_acpi_find_table("MCFG");
    if(mcfg) {
        k_debug("Found MCFG");
        __pci_conf.access_type = 2;
        __pci_conf.mcfg = mcfg;
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(mcfg, conf_spaces); i++) {
            k_debug("space: %#.16lx %d %d-%d", mcfg->conf_spaces[i].address, 
                                                 mcfg->conf_spaces[i].seg_group,
                                                 mcfg->conf_spaces[i].start_bus, 
                                                 mcfg->conf_spaces[i].end_bus);
            for(int bus = mcfg->conf_spaces[i].start_bus; bus <= mcfg->conf_spaces[i].end_bus; bus++) {
                __k_dev_probe_bus(bus);
            }
        }
    } else {
        for(int bus = 0; bus < 256; bus++) {
            __k_dev_probe_bus(bus);
        }
    }
}
