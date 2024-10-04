#ifndef __TYPES_LIST_H
#define __TYPES_LIST_H 1

#include <types/util.h>
#include <stddef.h>

struct _list;

typedef struct _list_node {
	struct _list_node* next;	
	struct _list_node* prev;	
	void* value;
	struct _list* owner;
} list_node;

typedef struct _list {
	list_node* head;
	list_node* tail;
	size_t size;
} list;

list*      list_create();
list_node* list_create_node(void*);
void  list_free(list*);
void  list_clear(list*);

void list_append(list*, list_node*);
void list_prepend(list*, list_node*);

list_node* list_push_back(list*, void*);
list_node* list_push_front(list*, void*);
list_node* list_pop_back(list*);
list_node* list_pop_front(list*);
list_node* list_insert_after(list*, list_node*, void*);
list_node* list_insert_before(list*, list_node*, void*);

list_node* list_get(list*, size_t);
void*      list_get_raw(list*, size_t);
#define list_get_type(list, type, i) (type) list_get_raw(list, i)

void  	   list_remove(list*, size_t);
void  	   list_delete(list*, list_node*);

list_node* list_find_cmp(list*, void*, comparator);
#define list_find(list, v) list_find_cmp(list, v, DEFAULT_COMPARATOR)

void list_swap(list_node*, list_node*);
void list_sort_cmp(list*, comparator);
#define list_sort(list) list_sort_cmp(list, DEFAULT_COMPARATOR)

#define foreach(element, list) \
	for(list_node* element = (list)->head; element != NULL; element = element->next)

#endif
