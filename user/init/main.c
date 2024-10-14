#include "dirent.h"
#include "fcntl.h"
#include "stdlib.h"
#include "sys/wait.h"
#include "unistd.h"
#include <stdio.h>
#include <string.h>

int exec_script(const char* script) {

    pid_t pid = fork();

    if(!pid) {
        char* const argv[] = {script, NULL};
        execv(script, argv);
        printf("Failed to execute: %s", script);
        exit(1);
    }

    int status = -1;
    waitpid(pid, &status, 0);

    return status;
}

int main(int argc, const char** argv) {
    open("/dev/null", O_RDONLY);
    open("/dev/log",  O_WRONLY);
    open("/dev/log",  O_WRONLY);

    printf("Welcome to WyrmOS!\r\n");
    DIR* d = opendir("/etc/init.d");
    if(!d) {
        printf("Failed to open /etc/init.d.");
    } else {
        struct dirent* dent = NULL;
        while((dent = readdir(d)) != NULL) {
            printf("Executing: %s\r\n", dent->name);
            char path[1024] = "/etc/init.d/";
            strncat(path, dent->name, 1024);
            int s = exec_script(path);
            if(s != 0) {
                printf("-- Failed! Code: %d", s);
            } else {
                printf("-- Ok");
            }
        }
    }

    while(1) { }
    return 0;
}
