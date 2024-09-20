#include <types/list.h>
#include <assert.h>
#include <stdlib.h>

list* list_create() {
	list* r = malloc(sizeof(list));
	r->head = NULL;
	r->tail = NULL;
	r->size = 0;
	return r;
}

void list_clear(list* list) {
	assert(list != NULL);
	while(list->head) {
		list_node* next = list->head->next;
		free(list->head);
		list->head = next;
	}
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void list_free(list* list) {
	list_clear(list);
	free(list);
}

static list_node* __list_create_node(void* v) {
	list_node* node = malloc(sizeof(list_node));
	node->next  = NULL;
	node->prev  = NULL;
	node->value = v;
	return node;
}

void list_append(list* l, list_node* n) {
	assert(l != NULL);
	n->owner = l;
	if(!l->tail) {
		l->head = n;
		l->tail = l->head;
	} else {
		l->tail->next = n;
		n->prev = l->tail;
		l->tail = n;
	}
	l->size++;
}

void list_prepend(list* l, list_node* n) {
	assert(l != NULL);
	if(!l->head) {
		list_append(l, n);
        return;
	} else {
		n->owner = l;
		l->head->prev = n;
		n->next = l->head;
		l->head = n;
	}
	l->size++;
}

list_node* list_push_back(list* l, void* v) {
	assert(l != NULL);
	list_node* n = __list_create_node(v);
	list_append(l, n);
	return n;
}

list_node* list_push_front(list* l, void* v) {
	assert(l != NULL);
	list_node* n = __list_create_node(v);
	list_prepend(l, n);
	return n;
}
list_node* list_pop_back(list* l) {
	assert(l != NULL);
	if(!l->tail) {
		return NULL;
	}
	list_node* n = l->tail;
	l->tail = n->prev;
	if(n == l->head) {
		l->head = l->tail;
	}
	if(l->tail) {
		l->tail->next = NULL;
	}
	n->next  	  = NULL;
	n->prev  	  = NULL;
	n->owner 	  = NULL;
    l->size--;
	return n;
}

list_node* list_pop_front(list* l) {
	assert(l != NULL);
	if(!l->head) {
		return NULL;
	}
	list_node* n = l->head;
	n->next  = NULL;
	n->prev  = NULL;
	n->owner = NULL;
	l->head = l->head->next;
	l->head->prev = NULL;
    l->size--;
	return n;
}

list_node* list_get(list* l, size_t i) {
	assert(l != NULL);
	if(i >= l->size) {
		return NULL;
	}
	list_node* n = l->head;
	while(i) {
		n = n->next;
		i--;
	}
	return n;
}

void* list_get_raw(list* l, size_t i) {
	assert(l != NULL);
	list_node* n = list_get(l, i);
	if(!n) {
		return NULL;
	} else {
		return n->value;
	}
}

void list_remove(list* l, size_t i) {
	assert(l != NULL);
	list_node* n = list_get(l, i);
	if(n) {
		list_delete(l, n);
	}
}

void list_delete(list* l, list_node* n) {
	assert(l != NULL);
	assert(n->owner == l);
	if(n->prev) {
		n->prev->next = n->next;
	}
	if(n->next) {
		n->next->prev = n->prev;
	}
    if(n == l->head) {
        l->head = n->next;
    }
    if(n == l->tail) {
        l->tail = n->prev;
    }
	free(n);
	l->size--;
}

list_node* list_find_cmp(list* l, void* v, comparator cmp) {
	assert(l != NULL);
	foreach(e, l) {
		if(cmp(e->value, v) == 0) {
			return e;
		}
	}
	return NULL;
}

void list_swap(list_node* a, list_node* b) {
    assert(a->owner == b->owner);
    if(a->prev) {
        a->prev->next = b;
    }
    if(a->next) {
        a->next->prev = b;
    }
    if(b->next) {
        b->next->prev = a;
    }
    if(b->prev) {
        b->prev->next = a;
    }
    void* tmp = a->next;
    a->next = b->next;
    b->next = tmp;
    tmp = a->prev;
    a->prev = b->prev;
    b->prev = tmp;

    if(!a->next) {
        a->owner->tail = a;
    } else if(!b->next) {
        b->owner->tail = b;
    }

    if(!a->prev) {
        a->owner->head = a;
    } else if(!b->prev) {
        b->owner->head = b;
    }
}

void list_sort_cmp(list* list, comparator cmp) {
    if(list->size == 0) {
        return;
    }
    list_node* a = list->head->next;
    while(a != NULL) {
        list_node* b = a;
        while(b->prev && cmp(b->prev->value, b->value) > 0) {
            list_swap(b->prev, b);
            b = b->prev;
        }
        a = a->next;
    }
}
