#ifndef __K_DEV_ACPI_H
#define __K_DEV_ACPI_H 1

#include <stdint.h>

typedef struct {
 char     signature[8];
 uint8_t  checksum;
 char     oemid[6];
 uint8_t  revision;
 uint32_t rsdt_address;
} __attribute__ ((packed)) acpi_rsdp;

typedef struct {
    acpi_rsdp rsdp;
    uint32_t  length;
    uint64_t  xsdt_address;
    uint8_t   extended_checksum;
    uint8_t   reserved[3];
} __attribute__ ((packed)) acpi_xsdp;

typedef struct {
  char     signature[4];
  uint32_t length;
  uint8_t  revision;
  uint8_t  checksum;
  char     oem_id[6];
  char     oem_table[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_rev;
}__attribute__ ((packed)) acpi_header;

#define ACPI_ENTRY_AMOUNT(root, table) \
    (((root)->header.length - sizeof(acpi_header)) / sizeof((root)->table[0]))

typedef struct {
    acpi_header header;
    uint32_t    tables[];
}__attribute__ ((packed)) acpi_rsdt;

typedef struct {
    acpi_header header;
    uint64_t    tables[];
}__attribute__ ((packed)) acpi_xsdt;

typedef struct {
    uint64_t address;
    uint16_t seg_group;
    uint8_t  start_bus;
    uint8_t  end_bus;
    uint32_t reserved;
}__attribute__ ((packed)) acpi_pci_space_entry;

typedef struct {
    acpi_header          header;
    uint64_t             reserved;
    acpi_pci_space_entry conf_spaces[];
}__attribute__ ((packed)) acpi_mcfg;

typedef struct {
    uint8_t type;
    uint8_t len;
    uint8_t data[];
}__attribute__((packed)) acpi_madt_entry;

typedef struct {
    uint8_t  id;
    uint8_t  apic_id;
    uint32_t flags;
}__attribute__((packed)) acpi_madt_lapic;

typedef struct {
    uint8_t  id;
    uint8_t  resv;
    uint32_t addr;
    uint32_t int_base;
}__attribute__((packed)) acpi_madt_ioapic;

typedef struct {
    uint8_t  bus;
    uint8_t  irq;
    uint32_t int_base;
    uint16_t flags; 
}__attribute__((packed)) acpi_madt_ioapic_iso;


typedef struct {
    uint8_t  id;
    uint16_t flags; 
    uint8_t  lint;
}__attribute__((packed)) acpi_madt_ioapic_nmi;

typedef struct {
    uint8_t  nmi;
    uint8_t  resv;
    uint16_t flags; 
    uint32_t int_base;
}__attribute__((packed)) acpi_madt_ioapic_nmi_src;

typedef struct {
    acpi_header     header;
    uint32_t        lapic;
    uint32_t        flags;
}__attribute__ ((packed)) acpi_madt;

acpi_header* k_dev_acpi_find_table(const char* table);

#endif
