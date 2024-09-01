#include <fs/path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

path* path_parse(const char* _path) {
	path* p = path_create();
	char* pathstr = strdup(_path);
	char* part = strtok(pathstr, "/");
	while(part) {
		size_t pl = strlen(part) - 1;
		if(part[pl] == '/') {
			part[pl] = '\0';
		}
		list_push_back(p, strdup(part));
		part = strtok(NULL, "/");
	}
	free(pathstr);
	return p;
}

void path_free(path* p) {
	foreach(pr, p) {
		free(pr->value);
	}
	list_free(p);
}

char* path_build(path* p) {
	size_t required_size = p->size + 1;
	foreach(pr, p) {
		required_size += strlen(pr->value);
	}
	char* path = malloc(required_size);
	foreach(pr, p) {
		strcat(path, pr->value);
	}
	return path;
}

char* path_filename(path* p) {
	return strdup(p->tail->value);
}

char* path_folder(path* p) {
	list_node* folder = list_pop_back(p);	
	char* path = path_build(p);
	list_append(p, folder);
	return path;
}

path* path_join(path* p, const char* part) {
	list_push_back(p, strdup(part));
	return p;
}

