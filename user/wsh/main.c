#include "unistd.h"
#include <stdio.h>

int main(int argc, const char** argv) {
    printf("wsh!\r\n");
    sleep(1);
    printf("exiting...\r\n");
    return 0;
}
