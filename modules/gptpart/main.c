#include "dev/log.h"
#include "fcntl.h"
#include "fs/fs.h"
#include <exec/module.h>
#include <stdlib.h>
#include <string.h>

DEFINE_MODULE("gptpart", load, unload)
// DEPENDS_ON("storage") -- TODO fix

typedef struct {
    char     signature[8];
    uint32_t rev;
    uint32_t size;
    uint32_t checksum;
    uint32_t reserved;
    uint64_t header_lba;
    uint64_t alt_lba;
    uint64_t first_usable_block;
    uint64_t last_usable_block;
    char     guid[16];
    uint64_t part_table_lba;
    uint32_t part_n;
    uint32_t entry_size;
    uint32_t part_table_checksum;
    uint8_t  pad[512 - 0x5C];
} __attribute__((packed)) gpt_header;

typedef struct {
    char type_guid[16];
    char uniq_guid[16];
    uint64_t start;
    uint64_t end;
    uint64_t attrib;
    char name[];
} gpt_part_entry;

typedef struct {
    fs_node*        dev;
    gpt_part_entry* part;
} gpt_part_meta;

static size_t __gpt_read(fs_node* dev, size_t offset, size_t size, uint8_t* buff) {
    gpt_part_meta* meta = dev->meta;
    return k_fs_read(meta->dev, meta->part->start * 512 + offset, size, buff);
}

static size_t __gpt_write(fs_node* dev, size_t offset, size_t size, uint8_t* buff) {
    gpt_part_meta* meta = dev->meta;
    return k_fs_write(meta->dev, meta->part->start * 512 + offset, size, buff);
}

static fs_node* __create_patrition_for(fs_node* dev, gpt_part_entry* ent, int n) {
    char name[32] = {0};
    snprintf(name, 32, "%s%d", dev->name, n);
    fs_node* res = k_fs_alloc_fsnode(name);
    gpt_part_meta* meta = malloc(sizeof(gpt_part_meta));
    meta->dev  = dev;
    meta->part = ent;
    res->meta = meta;
    res->ops.read  = __gpt_read;
    res->ops.write = __gpt_write;
    return res;
}

static int __probe_device(fs_node* dev) {
    gpt_header header;

    if(k_fs_read(dev, 512, 512, (void*) &header) != 512) {
        k_debug("Failed to read header");
        return 0;
    }

    if(memcmp(header.signature, "EFI PART", 8) != 0) {
        k_debug("Signature mismatch.", header.signature);
        return 0;
    }

    k_debug("Found GPT header.");
    k_debug("Revision: 0x%.4x", header.rev);

    size_t bytes_required = header.entry_size * header.part_n;
    void* entries = malloc(bytes_required);
    size_t read = k_fs_read(dev, 1024, bytes_required, (void*) entries);
    if(read != bytes_required) {
        k_error("Failed to read patritions.");
        return 0;
    }

    int n = 1;
    for(unsigned int i = 0; i < header.part_n; i++) {
        gpt_part_entry* entry = entries;
        if(entry->type_guid[0] == '\0') {
            continue;
        }
        k_debug("patrition: %ld - %ld", entry->start, entry->end);
        fs_node* p = __create_patrition_for(dev, entry, n);
        char path[128] = {0};
        snprintf(path, 128, "/dev/%s", p->name);
        k_fs_mount_node(path, p);
        entries += header.entry_size;
        n++;
    }

    return 1;
}

static char __sd_letter = 'a';

int load() {
    char sd_path[16] = "/dev/sd";
    sd_path[8] = '\0';
    while(__sd_letter <= 'z') {
        sd_path[7] = __sd_letter;
        fs_node* dev = k_fs_open(sd_path, O_RDONLY);
        if(dev) {
            if(!__probe_device(dev)) {
                k_fs_close(dev);
            }
        } else {
            break;
        }
        __sd_letter++;
    }
    k_info("Module loaded!");
    return 0;
}

int unload() {
    k_info("Module unloaded!");
    return 0;
}
