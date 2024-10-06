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


uint32_t k_dev_pci_read_dword(uint32_t dev, uint32_t offset) {
    assert(offset < 4096);
    if(__pci_conf.access_type == 0) {
        uint32_t addr = ((uint32_t) bus(dev) << 16) | 
            ((uint32_t) dev(dev) << 11) | 
            ((uint32_t) func(dev) << 8) | (offset & 0xFC)
            | ((uint32_t)0x80000000);
        outl(0xCF8, addr);
        return inl(0xCFC);
    } else if (__pci_conf.access_type == 2){
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(__pci_conf.mcfg, conf_spaces); i++) {
            if(bus(dev) >= __pci_conf.mcfg->conf_spaces[i].start_bus && 
               bus(dev) <= __pci_conf.mcfg->conf_spaces[i].end_bus) {
                void* mem = k_mem_iomap(__pci_conf.mcfg->conf_spaces[i].address + 
                        (((uintptr_t)bus(dev) << 20) | ((uintptr_t)dev(dev) << 15) | ((uintptr_t)func(dev) << 12)), 4096);
                return *(uint32_t*)(mem + (offset & 0xFC));
            }
        }
    }
    return 0;
}

uint16_t k_dev_pci_read_word(uint32_t dev, uint32_t offset) {
    uint32_t word = offset & 2; 
    return (k_dev_pci_read_dword(dev, offset) >> (word * 8)) & 0xFFFF;
}

uint8_t  k_dev_pci_read_byte(uint32_t dev, uint32_t offset) {
    uint32_t byte = offset & 1; 
    return (k_dev_pci_read_word(dev, offset) >> (byte * 8)) & 0xFF;
}

void k_dev_pci_write_dword(uint32_t dev, uint32_t offset, uint32_t value) {
    assert(offset < 4096);
    if(__pci_conf.access_type == 0) {
        uint32_t addr = ((uint32_t) bus(dev) << 16) | 
            ((uint32_t) dev(dev) << 11) | 
            ((uint32_t) func(dev) << 8) | (offset & 0xFC)
            | ((uint32_t)0x80000000);
        outl(0xCF8, addr);
        outl(0xCFC, value);
    } else if (__pci_conf.access_type == 2){
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(__pci_conf.mcfg, conf_spaces); i++) {
            if(bus(dev) >= __pci_conf.mcfg->conf_spaces[i].start_bus && 
               bus(dev) <= __pci_conf.mcfg->conf_spaces[i].end_bus) {
                void* mem = k_mem_iomap(__pci_conf.mcfg->conf_spaces[i].address + 
                        (((uintptr_t)bus(dev) << 20) | ((uintptr_t)dev(dev) << 15) | ((uintptr_t)func(dev) << 12)), 4096);
                *(uint32_t*)(mem + (offset & 0xFC)) = value;
            }
        }
    }
}

void k_dev_pci_write_word(uint32_t dev, uint32_t offset, uint16_t value) {
    assert(offset < 4096);
    if(__pci_conf.access_type == 0) {
       k_dev_pci_write_dword(dev, offset, value); 
    } else if (__pci_conf.access_type == 2){
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(__pci_conf.mcfg, conf_spaces); i++) {
            if(bus(dev) >= __pci_conf.mcfg->conf_spaces[i].start_bus && 
               bus(dev) <= __pci_conf.mcfg->conf_spaces[i].end_bus) {
                void* mem = k_mem_iomap(__pci_conf.mcfg->conf_spaces[i].address + 
                        (((uintptr_t)bus(dev) << 20) | ((uintptr_t)dev(dev) << 15) | ((uintptr_t)func(dev) << 12)), 4096);
                *(uint16_t*)(mem + (offset & 0xFC)) = value;
            }
        }
    }
}

void k_dev_pci_write_byte(uint32_t dev, uint32_t offset, uint8_t value) {
    assert(offset < 4096);
    if(__pci_conf.access_type == 0) {
       k_dev_pci_write_dword(dev, offset, value); 
    } else if (__pci_conf.access_type == 2){
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(__pci_conf.mcfg, conf_spaces); i++) {
            if(bus(dev) >= __pci_conf.mcfg->conf_spaces[i].start_bus && 
               bus(dev) <= __pci_conf.mcfg->conf_spaces[i].end_bus) {
                void* mem = k_mem_iomap(__pci_conf.mcfg->conf_spaces[i].address + 
                        (((uintptr_t)bus(dev) << 20) | ((uintptr_t)dev(dev) << 15) | ((uintptr_t)func(dev) << 12)), 4096);
                *(uint8_t*)(mem + (offset & 0xFC)) = value;
            }
        }
    }
}

pci_device* k_dev_pci_create_device(uint32_t address) {
    pci_device* dev = malloc(sizeof(pci_device));
    dev->address = address;
    return dev;
}

static void __k_dev_probe_bus(int bus);
static void __k_dev_probe_device(int bus, uint8_t device, uint8_t function) {
    uint32_t address = create_address(bus, device, function);
    uint16_t vendor = k_dev_pci_read_word(address, PCI_W_VENDOR);
    if(vendor == 0xFFFF) {
        return;
    }
    uint16_t dev = k_dev_pci_read_word(address, PCI_W_DEVICE);
    k_debug("%d:%d:%d: 0x%.4x 0x%.4x", bus, device, function, vendor, dev);
    list_push_back(__devices, k_dev_pci_create_device(address));
    if(function == 0) {
        uint8_t header_type = k_dev_pci_read_byte(address, PCI_B_HTYPE);
        if(header_type & 0x80) {
            for(int i = 1; i < 8; i++) {
                __k_dev_probe_device(bus, device, i);
            }
        }
    }
    uint8_t class    = k_dev_pci_read_byte(address, PCI_B_CLASS);
    uint8_t subclass = k_dev_pci_read_byte(address, PCI_B_SUBCLASS);
    if(class == 0x6 && subclass == 0x4) {
        k_debug("Bridge detected: 0x%.4x 0x%.4x", vendor, dev);
        __k_dev_probe_bus(k_dev_pci_read_byte(address, 0x19));
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

list*  k_dev_pci_find_devices(pci_scanner scanner) {
    list* r = list_create();
    foreach(node, __devices) {
        if(scanner(node->value)) {
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
