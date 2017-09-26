#include "client.h"

#include <stdio.h>
#include <stdlib.h>

char * serverAddress;
int serverPort;

int main(int argc, char ** argv) {
    switch(argc) {
        case 1:
            printf("Too few arguments provided. Usage: %s <server IP> <server port>\n", argv[0]);
            return 1;
        case 2:
            serverAddress = argv[1];
            serverPort = DEFAULT_PORT;
            break;
        case 3:
            serverAddress = argv[1];
            serverPort = atoi(argv[2]);
            break;
        default:
            printf("Too many arguments provided!\n");
            return 1;
    }

    validatePort(serverPort);

    if (serverAddress == NULL) {
        printf("Please provide a server address!\n");
        return 1;
    }

    printf("%s:%d", serverAddress, serverPort);
}