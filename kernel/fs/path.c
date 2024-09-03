#include "types/list.h"
#include <fs/path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <symbols.h>

path* path_create() {
	path* p  = malloc(sizeof(path));
	p->data  = list_create();
	p->flags = 0;
	return p;
}

path* path_parse(const char* _path) {
	path* p = path_create();
	char* pathstr = strdup(_path);
	char* part = strtok(pathstr, "/");
	while(part) {
		size_t pl = strlen(part) - 1;
		if(part[pl] == '/') {
			part[pl] = '\0';
		}
		list_push_back(p->data, strdup(part));
		part = strtok(NULL, "/");
	}
	free(pathstr);
	return p;
}

void path_free(path* p) {
	foreach(pr, p->data) {
		free(pr->value);
	}
	list_free(p->data);
	free(p);
}

char* path_build(path* p) {
	size_t required_size = p->data->size + 1;
	foreach(pr, p->data) {
		required_size += strlen(pr->value);
	}
	char* path = malloc(required_size);
	memset(path, 0, required_size);
	foreach(pr, p->data) {
		strcat(path, pr->value);
		strcat(path, "/");
	}
	if(!(p->flags & P_DIR)) {
		path[strlen(path) - 1] = '\0';
	}
	return path;
}

char* path_filename(path* p) {
	if(!p->data->tail) {
		return NULL;
	}
	return strdup(p->data->tail->value);
}

char* path_folder(path* p) {
	list_node* folder = list_pop_back(p->data);	
	char* path = path_build(p);
	list_append(p->data, folder);
	return path;
}

path* path_join(path* p, const char* part) {
	char* part_copy = strdup(part);
	if(part_copy[strlen(part) - 1] == '/') {
		p->flags |= P_DIR;
		part_copy[strlen(part) - 1] = '\0';
	} else {
		p->flags &= ~P_DIR;
	}
	list_push_back(p->data, part_copy);
	return p;
}

EXPORT(path_create)
EXPORT(path_build)
EXPORT(path_parse)
EXPORT(path_free)
EXPORT(path_join)
EXPORT(path_filename)
EXPORT(path_folder)
