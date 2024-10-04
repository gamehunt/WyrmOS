#ifndef __K_DEV_PCI_H
#define __K_DEV_PCI_H 1

#include <types/list.h>
#include <stdint.h>

#define PCI_W_VENDOR 0x0
#define PCI_W_DEVICE 0x2

#define PCI_B_SUBCLASS 0xA
#define PCI_B_CLASS    0xB
#define PCI_B_HTYPE    0xE

#define PCI_DW_BAR(n) (0x10 + (n) * 4)

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
} pci_device;

#define DEV_ADDR(d) d->bus, d->device, d->function

void     k_dev_pci_init();

uint32_t    k_dev_pci_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset);
uint16_t    k_dev_pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset);
uint8_t     k_dev_pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset);
pci_device* k_dev_pci_create_device(uint8_t bus, uint8_t device, uint8_t function);

list*  k_dev_pci_get_devices();
list*  k_dev_pci_find_devices(uint8_t class);

#endif
