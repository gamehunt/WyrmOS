#include <dev/acpi.h>
#include <stddef.h>
#include <string.h>
#include "boot/limine.h"
#include "dev/log.h"
#include "mem/paging.h"

__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

static acpi_rsdp* __rsdp = NULL;

static void __find_rsdp() {
    if(rsdp_request.response) {
        k_debug("RSDP provided by bootloader.");
        __rsdp = rsdp_request.response->address;
    } else {
        k_debug("RSDP not provided by bootloader, trying manual search.");

        for(uintptr_t addr = 0x000E0000; addr < 0x00200000; addr += 16) {
            const char* sig = (const char*) TO_VIRTUAL(addr);
            if(!strncmp(sig, "RSD PTR ", 8)) {
                __rsdp = (void*) sig;
                k_debug("RSDP found: %#.16lx", sig);
                break;
            }
        }

        if(!__rsdp) {
            k_debug("RSDP not found, giving up.");
        }
    }

    k_debug("RSDP Revision: %d", __rsdp->revision);
}

acpi_header* k_dev_acpi_find_table(const char* table) {
    if(!__rsdp) {
        __find_rsdp();
        if(!__rsdp) {
            __rsdp = (void*) 0x1;
        }
    }

    if((uintptr_t) __rsdp == 0x1) {
        k_warn("Ignoring ACPI request: no rsdp found.");
        return NULL;
    } 

    if(__rsdp->revision == 0) {
        acpi_rsdt* rsdt = (acpi_rsdt*) TO_VIRTUAL((uintptr_t) __rsdp->rsdt_address);
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(rsdt, tables); i++) {
            acpi_header* header = (void*) TO_VIRTUAL((uintptr_t) rsdt->tables[i]);
            if(!strcmp(header->signature, table)) {
                return header;
            }
        }
    } else {
        acpi_xsdp* xsdp = (acpi_xsdp*) __rsdp;
        acpi_xsdt* xsdt = (acpi_xsdt*) TO_VIRTUAL(xsdp->xsdt_address);
        for(size_t i = 0; i < ACPI_ENTRY_AMOUNT(xsdt, tables); i++) {
            acpi_header* header = (void*) TO_VIRTUAL(xsdt->tables[i]);
            if(!strncmp(header->signature, table, 4)) {
                return header;
            }
        }
    }

    return NULL;
}
