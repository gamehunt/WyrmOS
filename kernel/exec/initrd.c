#include "dev/log.h"
#include "fcntl.h"
#include "fs/fs.h"
#include "fs/path.h"
#include "symbols.h"
#include "types/list.h"
#include <exec/initrd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITRD_FSID 0x494E4954524400

struct tar_header
{
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
};

static uint64_t __ram_counter = 0;

static size_t __get_size(const char* _size) {
    size_t size  = 0;
    uint8_t j;
    size_t count = 1;
    for (j = 11; j > 0; j--, count *= 8) {
        size += ((_size[j - 1] - '0') * count);
    }
    return size;
}

static list* __parse_tar_module(fs_node* ram) {
    list* files = list_create();

    void* address = ram->meta;

    while(1)
    {
        struct tar_header *header = (struct tar_header *) address;
        if (header->filename[0] == '\0')
            break;

        list_push_back(files, header);

        size_t size = __get_size(header->size);

        address += ((size / 512) + 1) * 512;
        if (size % 512)
            address += 512;
    }

    return files;
}

static size_t __ram_read(fs_node* ram, size_t offset, size_t size, uint8_t* buffer) {
    if(offset > ram->size) {
        return 0;
    }

    if(offset + size > ram->size) {
        size = ram->size - offset;
        if(!size) {
            return 0;
        }
    }

    memcpy(buffer, ram->meta + offset, size);

    return size;
}

static int8_t __filename_comparator(void* a, void* b) {
    struct tar_header* file = a;
    char* name = b;
    return strcmp(file->filename, name);
}

static uint8_t __starts_with(const char* pre, const char* post) {
    return strncmp(pre, post, strlen(pre)) == 0;
}

static int __initrd_readdir(fs_node* root, struct dirent* dir, size_t index) {
    void* address = root->meta;
    struct tar_header* root_header = address;
    size_t c = 0;
    while(1) {
        struct tar_header* hdr = address;
        if(hdr->filename[0] == '\0') {
            break;
        }

        if(!__starts_with(root_header->filename, hdr->filename)) {
            break;
        }

        if(hdr != root_header) {
            if(c == index) {
                path* p = path_parse(hdr->filename);
                char* filename = path_filename(p);
                strncpy(dir->name, filename, DIRENT_NAME_LENGTH);
                free(filename);
                path_free(p);
                return 1;
            } else{
                c++;
            }
        }

        size_t size = __get_size(hdr->size);
        address += ((size / 512) + 1) * 512;
        if (size % 512)
            address += 512;
    }
    dir->name[0] = '\0';
    return 0;
}

static fs_node* __initrd_open(fs_node* root, path* p, uint16_t flags) {
    if(flags & O_DIR) {
        p->flags |= P_DIR;
    } else {
        p->flags &= ~P_DIR;
    }

    fs_node* r = NULL;
    char* path = path_build(p);

    list_node* file = list_find_cmp(root->meta, path, __filename_comparator);

    if(file) {
        struct tar_header* header = file->value;
        r       = k_fs_alloc_fsnode(p->data->tail->value);
        r->size = __get_size(header->size);
        if(p->flags & P_DIR) {
            r->meta   = (void*) header;
            r->flags |= FS_FL_DIR;
            r->ops.readdir = __initrd_readdir;
        } else {
            r->meta = (void*) header + 512;
            r->ops.read = __ram_read;
        }
    }

    free(path);
    return r;
}

static fs_node* __initrd_mount_callback(const char* name, fs_node* device) {
    fs_node* initrd  = k_fs_alloc_fsnode(name);
    initrd->meta     = __parse_tar_module(device);  
    initrd->ops.open = __initrd_open;
    initrd->fsid     = INITRD_FSID;
    return initrd;
}

void k_exec_initrd_init() {
    k_fs_register("initrd", __initrd_mount_callback);
}

int k_exec_initrd_load(void* address, size_t size) {
    char path[32] = {0};
    snprintf(path, 32, "/dev/ram%d", __ram_counter++);
    fs_node* node = k_fs_alloc_fsnode(&path[5]);
    node->size = size;
    node->meta = address;
    node->ops.read = __ram_read;
    k_fs_mount_node(path, node);
    return 0;
}
