#include "dirent.h"
#include "stdlib.h"
#include <stdio.h>

int main(int argc, const char** argv) {
    printf("Welcome to WyrmOS!\r\n");
    DIR* d = opendir("/etc/init.d");
    if(!d) {
        printf("Failed to open /etc/init.d.");
    } else {
        struct dirent* dent = NULL;
        while((dent = readdir(d)) != NULL) {
            printf("%s\r\n", dent->name);
        }
    }
    while(1) { }
    return 0;
}
