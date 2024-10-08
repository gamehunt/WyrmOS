#ifndef __K_FS_H
#define __K_FS_H 1

#define FS_NODE_NAME_LENGTH   256
#define FS_DIRENT_NAME_LENGTH 256
#define VFS_FSID 0x5646535f46534944

#include <stdint.h>
#include <stddef.h>

#include <fs/path.h>
#include <symbols.h>

typedef struct {
	char name[FS_DIRENT_NAME_LENGTH];
} dirent;

struct _fs_node;
typedef struct {
	struct _fs_node*(*open)(struct _fs_node* root, path*, uint16_t flags);
	void   (*close)(struct _fs_node*);
	size_t (*read)(struct _fs_node*, size_t, size_t, uint8_t*);
	size_t (*write)(struct _fs_node*, size_t, size_t, uint8_t*);
	int    (*readdir)(struct _fs_node*, dirent*, size_t);
} fs;

#define FS_FL_DIR (1 << 0)

typedef struct _fs_node{
	char      name[FS_NODE_NAME_LENGTH];
	size_t    size;
	void*     meta;
	uint16_t  flags;
	uint64_t  fsid;
	fs        ops;
} fs_node;

typedef fs_node*(*mount)(const char* name, fs_node* device);

INTERNAL void k_fs_init();

void     k_fs_create_mapping(const char* path);
fs_node* k_fs_mount(const char* path, const char* device, const char* type);
void     k_fs_mount_node(const char* path, fs_node* node);

fs_node* k_fs_open(const char* path, uint16_t flags);
void     k_fs_close(fs_node* node);
size_t   k_fs_read(fs_node* node, size_t offset, size_t bytes, uint8_t* buffer);
size_t   k_fs_write(fs_node* node, size_t offset, size_t bytes, uint8_t* buffer);
int      k_fs_readdir(fs_node* dir, dirent* dn, size_t index);

void     k_fs_register(const char* alias, mount mount_callback);
fs_node* k_fs_alloc_fsnode(const char* name);

void     k_fs_print_tree();

#endif
