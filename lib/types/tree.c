#include <types/tree.h>
#include <stdlib.h>

tree* tree_create(void* v) {
    tree* t     = malloc(sizeof(tree));
    t->children = list_create();
    t->value    = v;
    t->parent   = NULL;
    t->root     = t;
    return t;
}

void tree_free(tree* t) {
    if(t->parent) {
        tree_remove_child(t->parent, t);
    }
    foreach(c, t->children) {
        tree_free(c->value);
    }
    list_free(t->children);
    free(t);
}

void tree_append_child(tree* root, tree* child) {
    list_push_back(root->children, child);
    child->root   = root->root;
    child->parent = root;
}

tree*  tree_append(tree* root, void* v) {
    tree* t = tree_create(v);
    tree_append_child(root, t);
    return t;
}

void tree_remove_child(tree* root, tree* child) {
    list_node* n = list_find(root->children, child);
    list_delete(root->children, n);
    child->root   = NULL;
    child->parent = NULL;
}

tree* tree_find_child_cmp(tree* root, void* v, comparator cmp) {
    foreach(c, root->children) {
        if(cmp(((tree*)c->value)->value, v) == 0) {
            return c->value;
        }
    }
    return NULL;
}

tree*  tree_find_recursive_cmp(tree* root, void* v, comparator cmp) {
    if(cmp(root->value, v) == 0) {
        return root;
    }

    foreach(c, root->children) {
        tree* r = tree_find_recursive_cmp(root, v, cmp);
        if(r) {
            return r;
        }
    }

    return NULL;
}
