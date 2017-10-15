#include "common.h"

#include <stdio.h>
#include <stdlib.h>

int validatePort(int port) {
    if (port <= 0 || port >= 65535) {
        printf("Invalid port provided, %d. Please provide a port between 1 and %d", port, 65535);
        return 1;
    }

    return 0;
}

void error(char * msg) {
    perror(msg);
    exit(1);
}