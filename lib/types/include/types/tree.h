#ifndef __TYPES_TREE_H
#define __TYPES_TREE_H 1

#include <types/list.h>

struct _tree;

typedef struct _tree {
	void* value;
	list* children;
	struct _tree* parent;
	struct _tree* root;
} tree;

tree* tree_create(void*);
void  tree_free(tree*);

void   tree_append_child(tree* root, tree* child);
tree*  tree_append(tree* root, void* v);
void   tree_remove_child(tree* root, tree* child);

tree*  tree_find_child_cmp(tree* root, void* v, comparator cmp);
tree*  tree_find_recursive_cmp(tree* root, void* v, comparator cmp);

#define tree_find_child(r, v) tree_find_child_cmp(r, v, DEFAULT_COMPARATOR)
#define tree_find_cmp(r, v) tree_find_recursive_cmp(r, v, DEFAULT_COMPARATOR)

#endif
