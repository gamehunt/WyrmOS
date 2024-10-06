#ifndef __K_DEV_PCI_H
#define __K_DEV_PCI_H 1

#include <types/list.h>
#include <stdint.h>

#define PCI_W_VENDOR  0x0
#define PCI_W_DEVICE  0x2
#define PCI_W_COMMAND 0x4
#define PCI_W_STATUS  0x6

#define PCI_B_SUBCLASS 0xA
#define PCI_B_CLASS    0xB
#define PCI_B_HTYPE    0xE

#define PCI_DW_BAR(n) (0x10 + (n) * 4)

#define PCI_B_INT_LINE 0x3C

typedef struct {
    uint32_t address;
} pci_device;

#define device_address(dev) create_address(dev->bus, dev->device, dev->function)
#define create_address(bus, dev, func) ((uint32_t)((bus << 16) | (dev << 8) | (func)))
#define bus(addr)  ((uint8_t)((addr) >> 16))
#define dev(addr)  ((uint8_t)(addr >> 8))
#define func(addr) ((uint8_t)(addr))

typedef int(*pci_scanner)(pci_device* dev);

void     k_dev_pci_init();

uint32_t    k_dev_pci_read_dword(uint32_t dev, uint32_t offset);
uint16_t    k_dev_pci_read_word(uint32_t  dev, uint32_t offset);
uint8_t     k_dev_pci_read_byte(uint32_t  dev, uint32_t offset);
void        k_dev_pci_write_dword(uint32_t dev, uint32_t offset, uint32_t value);
void        k_dev_pci_write_word(uint32_t dev,  uint32_t offset, uint16_t value);
void        k_dev_pci_write_byte(uint32_t dev,  uint32_t offset, uint8_t value);
pci_device* k_dev_pci_create_device(uint32_t address);

list*  k_dev_pci_get_devices();
list*  k_dev_pci_find_devices(pci_scanner);

#endif
