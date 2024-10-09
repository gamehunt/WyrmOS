#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getenv(const char* env) {
    char** e = environ;
    while(e) {
        if(strstr(*e, env) == *e) {
            return &(*e)[strlen(env) + 1];
        }
        e++;
    }
    return NULL;
}

int setenv(const char *name, const char *value, int overwrite) {
    if(!overwrite) {
        if(getenv(name) != NULL) {
            return 0;
        }
    }
    char* tmp = malloc(strlen(name) + strlen(value) + 2);
    strcat(tmp, name);
    strcat(tmp, "=");
    strcat(tmp, value);
    return putenv(tmp);
}


extern int __envc;

int unsetenv(const char *name) {
    for(int i = 0; i < __envc; i++) {
        if(strstr(environ[i], name) == environ[i]) {
            if(i != __envc - 1) {
                environ[i] = environ[__envc - 1];
                environ[__envc - 1] = NULL;
            } else {
                environ[i] = NULL;
            }
            __envc--;
            break;
        }
    }
    return 0;
}

int putenv(char* string) {
    char* c = strchr(string, '='); 
    if(!c) {
        return 1;
    }

    *c = '\0';
    unsetenv(string);
    *c = '=';

    environ = realloc(environ, (__envc + 2) * sizeof(char*));
    environ[__envc]     = string;
    environ[__envc + 1] = NULL;
    __envc++;

    return 0;
}
