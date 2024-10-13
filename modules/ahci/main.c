#include "arch.h"
#include "mem/paging.h"
#include "proc/process.h"
#include "symbols.h"
#include "mem/pmm.h"
#include "types/list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __X86_64__
#include "cpu/interrupt.h"
#include "cpu/pic.h"
#endif
#include "dev/pci.h"
#include "fs/fs.h"
#include "mem/mmio.h"
#include <exec/module.h>

DEFINE_MODULE("ahci", load, unload)
PROVIDES("storage")

#define HBA_PxCMD_ST    0x0001
#define HBA_PxCMD_FRE   0x0010
#define HBA_PxCMD_FR    0x4000
#define HBA_PxCMD_CR    0x8000

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier

#define AHCI_DEV_NULL   0
#define AHCI_DEV_SATA   1
#define AHCI_DEV_SEMB   2
#define AHCI_DEV_PM     3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE  1
#define HBA_PORT_DET_PRESENT 3

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

#define ATA_SECTOR_SIZE 512

#define ATA_CMD_IDENTIFY    0xEC 
#define ATA_CMD_READ_DMA_EX 0x25 

typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

typedef struct tagFIS_REG_H2D
{
	uint8_t  fis_type;	// FIS_TYPE_REG_H2D
	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:3;	// Reserved
	uint8_t  c:1;		// 1: Command, 0: Control
	uint8_t  command;	// Command register
	uint8_t  featurel;	// Feature register, 7:0
	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;	// Device register
	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  featureh;	// Feature register, 15:8
	uint8_t  countl;	// Count register, 7:0
	uint8_t  counth;	// Count register, 15:8
	uint8_t  icc;		// Isochronous command completion
	uint8_t  control;	// Control register
	uint8_t  rsv1[4];	// Reserved
} FIS_REG_H2D;

typedef struct tagFIS_REG_D2H
{
	uint8_t  fis_type;    // FIS_TYPE_REG_D2H
	uint8_t  pmport:4;    // Port multiplier
	uint8_t  rsv0:2;      // Reserved
	uint8_t  i:1;         // Interrupt bit
	uint8_t  rsv1:1;      // Reserved
	uint8_t  status;      // Status register
	uint8_t  error;       // Error register
	uint8_t  lba0;        // LBA low register, 7:0
	uint8_t  lba1;        // LBA mid register, 15:8
	uint8_t  lba2;        // LBA high register, 23:16
	uint8_t  device;      // Device register
	uint8_t  lba3;        // LBA register, 31:24
	uint8_t  lba4;        // LBA register, 39:32
	uint8_t  lba5;        // LBA register, 47:40
	uint8_t  rsv2;        // Reserved
	uint8_t  countl;      // Count register, 7:0
	uint8_t  counth;      // Count register, 15:8
	uint8_t  rsv3[2];     // Reserved
	uint8_t  rsv4[4];     // Reserved
} FIS_REG_D2H;

typedef struct tagFIS_DATA
{
	uint8_t  fis_type;	// FIS_TYPE_DATA
	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:4;	// Reserved
	uint8_t  rsv1[2];	// Reserved
	uint32_t data[1];	// Payload
} FIS_DATA;

typedef struct tagFIS_PIO_SETUP
{
	uint8_t  fis_type;	// FIS_TYPE_PIO_SETUP
	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:1;	// Reserved
	uint8_t  d:1;		// Data transfer direction, 1 - device to host
	uint8_t  i:1;		// Interrupt bit
	uint8_t  rsv1:1;
	uint8_t  status;	// Status register
	uint8_t  error;		// Error register
	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;	// Device register
	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  rsv2;		// Reserved
	uint8_t  countl;	// Count register, 7:0
	uint8_t  counth;	// Count register, 15:8
	uint8_t  rsv3;		// Reserved
	uint8_t  e_status;	// New value of status register
	uint16_t tc;		// Transfer count
	uint8_t  rsv4[2];	// Reserved
} FIS_PIO_SETUP;

typedef struct tagFIS_DMA_SETUP
{
	uint8_t  fis_type;       // FIS_TYPE_DMA_SETUP
	uint8_t  pmport:4;       // Port multiplier
	uint8_t  rsv0:1;		 // Reserved
	uint8_t  d:1;            // Data transfer direction, 1 - device to host
	uint8_t  i:1;            // Interrupt bit
	uint8_t  a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed
    uint8_t  rsved[2];       // Reserved
    uint64_t DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
                             // SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
    uint32_t rsvd;           // More reserved
    uint32_t DMAbufOffset;   // Byte offset into buffer. First 2 bits must be 0
    uint32_t TransferCount;  // Number of bytes to transfer. Bit 0 must be 0
    uint32_t resvd;          // Reserved
} FIS_DMA_SETUP;

typedef volatile struct tagHBA_PORT
{
	uint32_t clb;		// 0x00, command list base address, 1K-byte aligned
	uint32_t clbu;		// 0x04, command list base address upper 32 bits
	uint32_t fb;		// 0x08, FIS base address, 256-byte aligned
	uint32_t fbu;		// 0x0C, FIS base address upper 32 bits
	uint32_t is;		// 0x10, interrupt status
	uint32_t ie;		// 0x14, interrupt enable
	uint32_t cmd;		// 0x18, command and status
	uint32_t rsv0;		// 0x1C, Reserved
	uint32_t tfd;		// 0x20, task file data
	uint32_t sig;		// 0x24, signature
	uint32_t ssts;		// 0x28, SATA status (SCR0:SStatus)
	uint32_t sctl;		// 0x2C, SATA control (SCR2:SControl)
	uint32_t serr;		// 0x30, SATA error (SCR1:SError)
	uint32_t sact;		// 0x34, SATA active (SCR3:SActive)
	uint32_t ci;		// 0x38, command issue
	uint32_t sntf;		// 0x3C, SATA notification (SCR4:SNotification)
	uint32_t fbs;		// 0x40, FIS-based switch control
	uint32_t rsv1[11];	// 0x44 ~ 0x6F, Reserved
	uint32_t vendor[4];	// 0x70 ~ 0x7F, vendor specific
} HBA_PORT;

typedef volatile struct tagHBA_MEM
{
	// 0x00 - 0x2B, Generic Host Control
	uint32_t cap;		// 0x00, Host capability
	uint32_t ghc;		// 0x04, Global host control
	uint32_t is;		// 0x08, Interrupt status
	uint32_t pi;		// 0x0C, Port implemented
	uint32_t vs;		// 0x10, Version
	uint32_t ccc_ctl;	// 0x14, Command completion coalescing control
	uint32_t ccc_pts;	// 0x18, Command completion coalescing ports
	uint32_t em_loc;	// 0x1C, Enclosure management location
	uint32_t em_ctl;	// 0x20, Enclosure management control
	uint32_t cap2;		// 0x24, Host capabilities extended
	uint32_t bohc;		// 0x28, BIOS/OS handoff control and status
	// 0x2C - 0x9F, Reserved
	uint8_t  rsv[0xA0-0x2C];
	// 0xA0 - 0xFF, Vendor specific registers
	uint8_t  vendor[0x100-0xA0];
	// 0x100 - 0x10FF, Port control registers
	HBA_PORT	ports[1];	// 1 ~ 32
} HBA_MEM;

typedef uint64_t FIS_DEV_BITS;

typedef volatile struct tagHBA_FIS
{
	FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
	uint8_t         pad0[4];
	FIS_PIO_SETUP	psfis;		// PIO Setup FIS
	uint8_t         pad1[12];
	FIS_REG_D2H	    rfis;		// Register – Device to Host FIS
	uint8_t         pad2[4];
	FIS_DEV_BITS	sdbfis;		// Set Device Bit FIS
	uint8_t         ufis[64];
	uint8_t   	    rsv[0x100-0xA0];
} HBA_FIS;

typedef struct tagHBA_CMD_HEADER
{
	uint8_t  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
	uint8_t  a:1;		// ATAPI
	uint8_t  w:1;		// Write, 1: H2D, 0: D2H
	uint8_t  p:1;		// Prefetchable
	uint8_t  r:1;		// Reset
	uint8_t  b:1;		// BIST
	uint8_t  c:1;		// Clear busy upon R_OK
	uint8_t  rsv0:1;		// Reserved
	uint8_t  pmp:4;		// Port multiplier port
	uint16_t prdtl;		// Physical region descriptor table length in entries
	volatile
	uint32_t prdbc;		// Physical region descriptor byte count transferred
	uint32_t ctba;		// Command table descriptor base address
	uint32_t ctbau;		// Command table descriptor base address upper 32 bits
	uint32_t rsv1[4];	// Reserved
} HBA_CMD_HEADER;

typedef struct tagHBA_PRDT_ENTRY
{
	uint32_t dba;		// Data base address
	uint32_t dbau;		// Data base address upper 32 bits
	uint32_t rsv0;		// Reserved
	uint32_t dbc:22;		// Byte count, 4M max
	uint32_t rsv1:9;		// Reserved
	uint32_t i:1;		// Interrupt on completion
} HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL
{
	uint8_t  cfis[64];	// Command FIS
	uint8_t  acmd[16];	// ATAPI command, 12 or 16 bytes
	uint8_t  rsv[48];	// Reserved
	HBA_PRDT_ENTRY	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
} HBA_CMD_TBL;

typedef struct {
	uint16_t flags;
	uint16_t unused1[9];
	char     serial[20];
	uint16_t unused2[3];
	char     firmware[8];
	char     model[40];
	uint16_t sectors_per_int;
	uint16_t unused3;
	uint16_t capabilities[2];
	uint16_t unused4[2];
	uint16_t valid_ext_data;
	uint16_t unused5[5];
	uint16_t size_of_rw_mult;
	uint32_t sectors_28;
	uint16_t unused6[38];
	uint64_t sectors_48;
	uint16_t unused7[152];
} __attribute__((packed)) ata_identify_t;

typedef struct {
	HBA_MEM*  memory;
    int       port;
	list*     waiters;
    list*     cmd_waiters;
} ahci_device;

#define ncs(port) ((uint8_t)(port->cap >> 8) & 0xF)

static list* __devices = NULL;

static int __ahci_type(HBA_PORT* port) {
	uint32_t ssts = port->ssts;

	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

static const char* __typestr(int t) {
    switch(t){
        case AHCI_DEV_SATA:
            return "SATA drive";
        case AHCI_DEV_SATAPI:
            return "SATAPI drive";
        case AHCI_DEV_SEMB:
            return "SEMB drive";
        case AHCI_DEV_PM:
            return "PM drive";
        default:
            return "Unknown / No drive";
    }
}

static ahci_device* __create_device(HBA_MEM* mem, int port) {
	ahci_device* device = malloc(sizeof(ahci_device));
	device->memory      = mem;
	device->port        = port;
	device->waiters     = list_create();
    device->cmd_waiters = list_create();
	return device;
}

static char* __fix_ata_string(char* string, int len) {
    for(int i = 0; i < len - 1; i += 2) {
        char tmp = string[i];
        string[i] = string[i + 1]; 
        string[i + 1] = tmp;
    }

    int cr = 0;
    for(int i = 0; i < len; i++) {
        if(string[i] == ' ' && string[i + 1] == ' ') {
            cr = 1;
            string[i] = '\0';
            break;
        }
    }

    if(!cr) {
        string[len - 1] = '\0';
    }

    return string;
}

static void __port_start_cmd(HBA_PORT *port)
{
	while (port->cmd & HBA_PxCMD_CR)
		;

	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}

static void __port_stop_cmd(HBA_PORT *port)
{
	port->cmd &= ~HBA_PxCMD_ST;
	port->cmd &= ~HBA_PxCMD_FRE;

	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
}

static int __get_cmd_slot(HBA_PORT* port) {
    uint32_t slots = port->sact | port->ci;
    for(int i = 0; i < 32; i++) {
        if((slots & 1) == 0) {
            return i;
        }
        slots >>= 1;
    }
    return -1;
}

#define ident_failure(msg) \
		k_error("Port is hung\n"); \
        k_mem_free_dma(buffer, 512); \
        buffer = NULL; \
        goto end; \

static uint16_t* __ahci_identify(HBA_PORT* port) {
    port->is = (uint32_t) -1;
    int slot = __get_cmd_slot(port);

    if(slot == -1) {
        k_error("Failed to get free command slot");
        return NULL;
    }

    uint64_t buffer_phys = 0;
    void* buffer = k_mem_alloc_dma(512, &buffer_phys);
    memset(buffer, 0, 512);

    volatile HBA_CMD_HEADER* command_list  = k_mem_iomap(port->clb, sizeof(HBA_CMD_HEADER) * 32);

    command_list[slot].cfl   = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    command_list[slot].w     = 0;
    command_list[slot].prdtl = 1;

    uint64_t table_phys;
    HBA_CMD_TBL* table = k_mem_alloc_dma(sizeof(HBA_CMD_TBL), &table_phys);
    memset(table, 0, sizeof(HBA_CMD_TBL));

    command_list[slot].ctba  = (uint32_t) (table_phys);
    command_list[slot].ctbau = (uint32_t) (table_phys >> 32);

	table->prdt_entry[0].dba = (uint32_t) buffer_phys;
	table->prdt_entry[0].dbc = 511;	
	table->prdt_entry[0].i   = 1;

    FIS_REG_H2D* cmdfis = (FIS_REG_H2D*)(&table->cfis);
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c        = 1;
    cmdfis->command  = ATA_CMD_IDENTIFY;
    cmdfis->device   = 0;

    int spin = 0;
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		ident_failure("Port is hung\n");
	}

    port->ci = (1 << slot);

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1 << slot)) == 0) {
			break;
        }
		if (port->is & (1 << 5)) // Task file error
		{
			ident_failure("Read disk error\n");
		}
	}

	// Check again
	if (port->is & (1 << 5))
	{
		ident_failure("Read disk error\n");
	}

end:
    k_mem_free_dma(table, sizeof(HBA_CMD_TBL));

    return buffer;
}

static char __letter = 'a';
static size_t __ahci_read(fs_node* dev, size_t start, size_t bytes, uint8_t* buffer);

static fs_node* __create_fsnode(ahci_device* dev) {
    char name[] = {'s', 'd', __letter++, '\0'};
    fs_node* node = k_fs_alloc_fsnode(name);
    node->ops.read = __ahci_read;
    node->meta = dev;
    return node;
}

static void __init_port(HBA_MEM* mem, int _port) {
    HBA_PORT* port = &mem->ports[_port];

    __port_stop_cmd(port);

    uint64_t command_frames; 
    volatile HBA_CMD_HEADER* command_list = k_mem_alloc_dma(sizeof(HBA_CMD_HEADER) * 32, &command_frames);
    port->clbu = (uint32_t) (command_frames >> 32);
    port->clb  = (uint32_t) (command_frames);

    uint64_t fis_frames;
    volatile HBA_FIS* fis = k_mem_alloc_dma(sizeof(HBA_FIS), &fis_frames); 
    port->fbu = (uint32_t) (fis_frames >> 32);
    port->fb  = (uint32_t) (fis_frames);

    memset((void*) command_list, 0, sizeof(HBA_CMD_HEADER) * 32);
    memset((void*) fis, 0, sizeof(HBA_FIS));

    __port_start_cmd(port);

    ata_identify_t* ident = (ata_identify_t*) __ahci_identify(port);
    if(!ident) {
        k_error("Ident error.");
        return;
    }

    char* model  = __fix_ata_string(ident->model,   40);
    char* firmwr = __fix_ata_string(ident->firmware, 8);
    char* serial = __fix_ata_string(ident->serial,  20);

    k_debug("%s %s %s", model, firmwr, serial);
    k_debug("LBA28 size: %dMB", ident->sectors_28 / ATA_SECTOR_SIZE);
    k_debug("LBA48 size: %dMB", ident->sectors_48 / ATA_SECTOR_SIZE);

    k_mem_free_dma(ident, 512);

    fs_node* node = __create_fsnode(__create_device(mem, _port));
    node->size = (ident->sectors_48 ? ident->sectors_48 : ident->sectors_28) * ATA_SECTOR_SIZE;

	list_push_back(__devices, node->meta);

    port->ie = 0x7dc000ff; // Enable all interrupts

    char path[32] = {0};
    snprintf(path, 32, "/dev/%s", node->name);
    k_fs_mount_node(path, node);
}

// TODO map non-dma buffer for this?
static HBA_CMD_TBL* __ahci_init_prdt_for_buffer(volatile HBA_CMD_HEADER* cmd, void* buffer, size_t size) {
	cmd->prdtl = size / KB(8) + !!(size % KB(8));	
    uint64_t table_phys;
    HBA_CMD_TBL* table = k_mem_alloc_dma(sizeof(HBA_CMD_TBL) + (cmd->prdtl - 1) * sizeof(HBA_PRDT_ENTRY), &table_phys);
    cmd->ctba  = (uint32_t) (table_phys);
    cmd->ctbau = (uint32_t) (table_phys >> 32);
    memset(table, 0, sizeof(HBA_CMD_TBL) + (cmd->prdtl - 1) * sizeof(HBA_PRDT_ENTRY));
	for(size_t i = 0; i < cmd->prdtl; i++) {
		uint64_t phys = k_mem_paging_get_physical((uintptr_t) buffer + i * KB(8));
		table->prdt_entry[i].dba  = (uint32_t)((uintptr_t) phys);	
		table->prdt_entry[i].dbau = (uint32_t)((uintptr_t) phys >> 32);	
		table->prdt_entry[i].dbc  = size >= KB(8) ? KB(8) : size;
		table->prdt_entry[i].i    = (i == (cmd->prdtl - 1));
        size -= i * KB(8);
	}
    return table;
}

#define device_port(dev) (&dev->memory->ports[dev->port])

static size_t __ahci_read_sectors(ahci_device* device, size_t start, size_t sectors, uint8_t* buffer) {
    HBA_PORT* port = device_port(device);

    port->is = (uint32_t) -1;
    int slot = -1;

    do {
        slot = __get_cmd_slot(port);
        if(slot == -1) {
            k_process_sleep_on_queue(device->cmd_waiters);
        } else {
            break;
        }
    } while(1);

    volatile HBA_CMD_HEADER* command_list  = k_mem_iomap(port->clb, sizeof(HBA_CMD_HEADER) * 32);

    command_list[slot].cfl   = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    command_list[slot].w     = 0;

    HBA_CMD_TBL* table = __ahci_init_prdt_for_buffer(&command_list[slot], buffer, sectors * ATA_SECTOR_SIZE);

	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&table->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;

	cmdfis->lba0 = (uint8_t) start;
	cmdfis->lba1 = (uint8_t)(start >> 8);
	cmdfis->lba2 = (uint8_t)(start >> 16);
	cmdfis->device = 1 << 6; // LBA mode

	cmdfis->lba3 = (uint8_t)(start >> 24);
	cmdfis->lba4 = (uint8_t)(start >> 32);
	cmdfis->lba5 = (uint8_t)(start >> 48);

	cmdfis->countl = sectors & 0xFF;
	cmdfis->counth = (sectors >> 8) & 0xFF;

    int spin = 0;

    // The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}

	if (spin == 1000000)
	{
		k_error("Port is hung");
        command_list[slot].ctba  = 0; // IDK, what kind of cleanup we need there?
        command_list[slot].ctbau = 0;
        k_mem_free_dma(table, sizeof(HBA_CMD_TBL) + sizeof(HBA_PRDT_ENTRY) * (command_list[slot].prdtl - 1));
		return 0;
	}

	port->ci = 1 << slot; // Issue command

    while(port->ci & (1 << slot)) {
	    k_process_sleep_on_queue(device->waiters);
        if(port->is & (1 << 5)) {
            k_error("I/O error");
            return 0;
        }
    }

	return sectors;
}

static size_t __ahci_read(fs_node* dev, size_t start, size_t bytes, uint8_t* buffer) {
	ahci_device* d = dev->meta;

	size_t originalSize   = bytes;
	size_t originalOffset = start;

	if(bytes < ATA_SECTOR_SIZE) {
		bytes = ATA_SECTOR_SIZE;
	}

	if(bytes % ATA_SECTOR_SIZE) {
		bytes = ALIGN(bytes, ATA_SECTOR_SIZE);
	}

	void* tmp = k_mem_alloc_dma(bytes, NULL);
	size_t sectors = __ahci_read_sectors(d, start / ATA_SECTOR_SIZE, bytes / ATA_SECTOR_SIZE, tmp);
	if(sectors * ATA_SECTOR_SIZE < bytes) {
		originalSize = sectors * ATA_SECTOR_SIZE;
	}

	memcpy(buffer, tmp + (start % ATA_SECTOR_SIZE), originalSize);
	k_mem_free_dma(tmp, bytes);

    k_process_wakeup_queue(d->cmd_waiters);

	return originalSize;
}

static void __ahci_irq(regs* r) {
    IRQ_ACK(INT_TO_IRQ(r->int_no));
    foreach(_dev, __devices) {
        ahci_device* dev = _dev->value;  
        if(dev->memory->is & (1 << dev->port)) {
            k_process_wakeup_queue(dev->waiters);
        }
    }
}

static void __try_init_ahci(pci_device* dev) {
    k_debug("Trying to initialize AHCI device %#.6x", dev->address);

    uint16_t cmd = k_dev_pci_read_word(dev->address, PCI_W_COMMAND);
    cmd |= (1 << 1);  // Memory space
    cmd |= (1 << 2);  // Bus master
    cmd &= ~(1 << 10); // Interrupt enable
    k_dev_pci_write_word(dev->address, PCI_W_COMMAND, cmd);

    uint8_t  intr = k_dev_pci_read_byte(dev->address, PCI_B_INT_LINE);
    uint32_t bar  = k_dev_pci_read_dword(dev->address, PCI_DW_BAR(5)) & 0xFFFFFFF0;

    k_cpu_int_setup_irq_handler(intr, __ahci_irq);

    k_debug("pci bar5 = %#.8x", bar);
    k_debug("pci int  = %d", intr);

    HBA_MEM* mem = k_mem_iomap(bar, 0x1100);

    k_debug("ports impl = %#x", mem->pi);
    k_debug("version    = %d.%d%d", (mem->vs >> 16) & 0xFF,
                                    (mem->vs >> 8) & 0XFF,
                                    (mem->vs) & 0xFF
    );

    mem->ghc |= (1 << 1) | (1 << 31); // AHCI enable & INT enable

    for(int i = 0; i < 32; i++) {
        if(!(mem->pi & (1 << i))) {
            continue;
        }

        int type = __ahci_type(&mem->ports[i]);

        if(type == AHCI_DEV_NULL) {
            continue;
        }

        k_debug("Port %d: drive=%s, status=%d", i, __typestr(type), mem->ports[i].ssts);

        if(type == AHCI_DEV_SATA) {
            __init_port(mem, i);
        }
    }
}

static int __ahci_pci_scanner(pci_device* dev) {
    uint8_t class    = k_dev_pci_read_byte(dev->address, PCI_B_CLASS);
    uint8_t subclass = k_dev_pci_read_byte(dev->address, PCI_B_SUBCLASS);
    return class == 0x1 &&
           subclass == 0x6;
}

int load() {
    list* storages = k_dev_pci_find_devices(__ahci_pci_scanner);
    if(!storages->size) {
        k_debug("No AHCI devices found.");
    }
	__devices = list_create();
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
