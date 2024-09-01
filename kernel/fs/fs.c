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
	vfs_node* b = (vfs_node*) _b;	
	return strcmp(a->name, b->name);
}

static int8_t __callback_comparator(void* a, void* b) {
	return strcmp(((mount_callback*) a)->alias, b);
}

static fs_node* __open_vfs(vfs_node* _node) {
	fs_node* node = malloc(sizeof(fs_node));
	memset(node, 0, sizeof(fs_node));
	strcpy(node->name, _node->name);
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
	foreach(c, p) {
		tree* next = tree_find_child_cmp(parent, c->value, __vfs_node_comparator);
		if(next) {
			parent = next;
		} else {
			while(c) {
				list_push_back(left, strdup(c->value));
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

	foreach(c, left) {
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

fs_node* k_fs_mount(const char* _path, const char* device, const char* type) {
	fs_node* dev = k_fs_open(device);

	if(!dev) {
		return NULL;
	}

	mount callback = __get_mount_callback(type);
	if(!callback) {
		return NULL;
	}

	vfs_node* node = __k_fs_create_mapping(_path);
	node->root = dev;

	path* path = path_parse(_path);
	char* name = path_filename(path);

	fs_node* mountpoint = callback(name, dev);

	free(name);
	path_free(path);

	return mountpoint;
}

fs_node* k_fs_open(const char* _path) {
	path* p = path_parse(_path);
	path* left = path_create();
	vfs_node* mountpoint = __get_mountpoint(p, left);
	fs_node*  result = NULL;
	if(mountpoint) {
		if(left->size == 0) {
			if(!mountpoint->root) {
				result = __open_vfs(mountpoint);
			} else {
				result = mountpoint->root;
			}
		} else if(mountpoint->root){
			result = mountpoint->root->ops.open(left);
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
