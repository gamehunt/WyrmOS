#include "dev/log.h"
#include "symbols.h"
#include "types/list.h"
#include <fs/fs.h>
#include <fs/path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types/tree.h>

typedef struct {
	char     name[FS_NODE_NAME_LENGTH];	
	tree*    owner;
	fs_node* root;
} vfs_node;

typedef struct {
	const char* alias;
	mount callback;
} mount_callback;

static tree* __vfs_tree;
static list* __mount_callbacks;

static int8_t __vfs_node_comparator(void* _a, void* _b) {
	vfs_node* a = (vfs_node*) _a;	
	char* b = (char*) _b;	
	return strcmp(a->name, b);
}

static int8_t __callback_comparator(void* a, void* b) {
	return strcmp(((mount_callback*) a)->alias, b);
}

static fs_node* __open_vfs(vfs_node* _node) {
	fs_node* node = k_fs_alloc_fsnode(_node->name);
	node->meta = _node;
	node->size = 0;
	node->fsid = VFS_FSID;
	return node;
}

static vfs_node* __alloc_vfs_node(const char* name) {
	vfs_node* n = malloc(sizeof(vfs_node));
	memset(n, 0, sizeof(vfs_node));
	strncpy(n->name, name, FS_NODE_NAME_LENGTH);
	return n;
}

static vfs_node* __get_mountpoint(path* p, path* left) {
	tree* parent = __vfs_tree;
	foreach(c, p->data) {
		tree* next = tree_find_child_cmp(parent, c->value, __vfs_node_comparator);
		if(next) {
			parent = next;
		} else {
			while(c) {
				list_push_back(left->data, strdup(c->value));
				c = c->next;
			}
			break;
		}
	}	
	return (vfs_node*) parent->value;
}

static void __k_fs_print_vfs_node(vfs_node* node, int depth) {
	for(int i = 0; i < depth; i++) {
		printf("--");
	}
	printf("%s\r\n", node->name);
	foreach(c, node->owner->children) {
		__k_fs_print_vfs_node(((tree*)c->value)->value, depth + 1);
	}
}

void k_fs_print_tree() {
	__k_fs_print_vfs_node(__vfs_tree->value, 0);
}

void k_fs_init() {
	vfs_node* root = __alloc_vfs_node("[root]");
	__vfs_tree = tree_create(root);
	root->owner = __vfs_tree;
	__mount_callbacks = list_create();
}

void k_fs_register(const char* alias, mount callback) {
	mount_callback* cb = malloc(sizeof(mount_callback));
	cb->alias = alias;
	cb->callback = callback;
	list_push_back(__mount_callbacks, cb);
}

static mount __get_mount_callback(const char* alias) {
	list_node* cb = list_find_cmp(__mount_callbacks, (void*) alias, __callback_comparator);
	if(cb) {
		return ((mount_callback*)cb->value)->callback;
	}
	return NULL;
}

static vfs_node* __k_fs_create_mapping(const char* _path) {
	path* p    = path_parse(_path);
	path* left = path_create();

	vfs_node* parent = __get_mountpoint(p, left);
	tree* tree_parent = parent->owner;

	foreach(c, left->data) {
		vfs_node* child = __alloc_vfs_node(c->value);
		tree_parent = tree_append(tree_parent, child);
		child->owner = tree_parent;
	}

	path_free(left);
	path_free(p);

	return tree_parent->value;
}

void k_fs_create_mapping(const char *path) {
	__k_fs_create_mapping(path);
}

void k_fs_mount_node(const char* path, fs_node* node) {
	vfs_node* n = __k_fs_create_mapping(path);
	if(n->root != NULL) {
		k_error("%s: already mounted.", path);
		return;
	}
	n->root = node;
	k_info("Mounted: %s", path);
}

fs_node* k_fs_mount(const char* _path, const char* device, const char* type) {
	fs_node* dev = k_fs_open(device, FS_O_RW);

	if(!dev) {
		k_error("%s: device not found", device);
		return NULL;
	}

	mount callback = __get_mount_callback(type);
	if(!callback) {
		k_error("Attempted to mount %s to %s with unknown fs type %s.\r\n", _path, device, type);
		return NULL;
	}

	vfs_node* node = __k_fs_create_mapping(_path);
	if(node->root != NULL) {
		k_error("%s: already mounted.", _path);
		return NULL;
	}

	node->root = callback(node->name, dev);

	return node->root;
}

static fs_node* __dup(fs_node* node) {
    fs_node* new = malloc(sizeof(fs_node));
    memcpy(new, node, sizeof(fs_node));
    return new;
}

fs_node* k_fs_open(const char* _path, uint16_t flags) {
	path* p = path_parse(_path);
	path* left = path_create();
	vfs_node* mountpoint = __get_mountpoint(p, left);
	fs_node*  result = NULL;
	if(mountpoint) {
		if(left->data->size == 0) {
			if(!mountpoint->root) {
				result = __open_vfs(mountpoint);
			} else {
				result = __dup(mountpoint->root);
			}
		} else if(mountpoint->root && mountpoint->root->ops.open){
			result = mountpoint->root->ops.open(mountpoint->root, left, flags);
		}
	}
	path_free(left);
	path_free(p);
	return result;
}

void k_fs_close(fs_node* node) {
	if(node->ops.close) {
		node->ops.close(node);
	}
	free(node);
}

size_t k_fs_read(fs_node* node, size_t offset, size_t bytes, uint8_t* buffer) {
	if(node->ops.read) {
		return node->ops.read(node, offset, bytes, buffer);
	}
	return 0;
}

size_t k_fs_write(fs_node* node, size_t offset, size_t bytes, uint8_t* buffer) {
	if(node->ops.write) {
		return node->ops.write(node, offset, bytes, buffer);
	}
	return 0;
}

int k_fs_readdir(fs_node* dir, dirent* dn, size_t index) {
	if(dir->ops.readdir) {
		return dir->ops.readdir(dir, dn, index);
	}
	return 0;
}

fs_node*  k_fs_alloc_fsnode(const char* name) {
	fs_node* node = malloc(sizeof(fs_node));
	memset(node, 0, sizeof(fs_node));
	strncpy(node->name, name, FS_NODE_NAME_LENGTH);
	return node;
}
