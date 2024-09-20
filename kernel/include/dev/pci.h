#ifndef __K_DEV_PCI_H
#define __K_DEV_PCI_H 1

#include <stdint.h>

#define PCI_W_VENDOR 0x0
#define PCI_W_DEVICE 0x2

#define PCI_B_SUBCLASS 0xA
#define PCI_B_CLASS    0xB
#define PCI_B_HTYPE    0xE

void     k_dev_pci_init();

uint32_t k_dev_pci_read_dword(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset);
uint16_t k_dev_pci_read_word(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset);
uint8_t  k_dev_pci_read_byte(uint8_t bus, uint8_t device, uint8_t function, uint32_t offset);

#endif
