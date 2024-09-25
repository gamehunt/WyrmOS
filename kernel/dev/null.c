#include "fs/fs.h"
#include <stddef.h>
#include <string.h>

static size_t __k_dev_read_zero(fs_node* node, size_t offset, size_t size, uint8_t* buffer) {
    memset(buffer, 0, size);
    return size;
}

static size_t __k_dev_write_zero(fs_node* node, size_t offset, size_t size, uint8_t* buffer) {
    return size;
}

static size_t __k_dev_read_null(fs_node* node, size_t offset, size_t size, uint8_t* buffer) {
    return 0;
}

static size_t __k_dev_write_null(fs_node* node, size_t offset, size_t size, uint8_t* buffer) {
    return 0;
}

static fs_node* __k_dev_create_zero() {
    fs_node* node = k_fs_alloc_fsnode("zero");
    node->ops.write = __k_dev_write_zero;
    node->ops.read  = __k_dev_read_zero;
    return node;
}

static fs_node* __k_dev_create_null() {
    fs_node* node = k_fs_alloc_fsnode("null");
    node->ops.write = __k_dev_write_null;
    node->ops.read  = __k_dev_read_null;
    return node;
}

void k_dev_init_null() {
    k_fs_mount_node("/dev/null", __k_dev_create_null());
    k_fs_mount_node("/dev/zero", __k_dev_create_zero());
}
