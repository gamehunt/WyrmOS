#include "dirent.h"
#include <stdio.h>

int main() {
    printf("Welcome to WyrmOS!\r\n");
    DIR* d = opendir("/modules");
    if(!d) {
        printf("Failed to open dir.");
    } else {
        struct dirent* dent = NULL;
        while((dent = readdir(d)) != NULL) {
            printf("%s\r\n", dent->name);
        }
    }
    while(1) { 
    }
    return 0;
}
