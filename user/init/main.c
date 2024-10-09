#include "dirent.h"
#include "stdlib.h"
#include "unistd.h"
#include <stdio.h>

int main(int argc, const char** argv) {
    printf("Welcome to WyrmOS! %s\r\n", argv[0]);
    DIR* d = opendir("/modules");
    if(!d) {
        printf("Failed to open dir.");
    } else {
        struct dirent* dent = NULL;
        while((dent = readdir(d)) != NULL) {
            printf("%s\r\n", dent->name);
        }
    }
    if(!fork()) {
        char* _argv[] = {"wsh", NULL};
        execv("/bin/wsh", _argv);
    }
    while(1) { }
    return 0;
}
