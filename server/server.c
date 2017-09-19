#include "server.h"

int serverPort;

int main(int argc, char ** argv) {
    switch(argc) {
        case 1:
            serverPort = DEFAULT_PORT;
            break;
        case 2:
            serverPort = atoi(argv[1]);
            break;
        default:
            printf("Too many arguments provided!\n");
            return 1;
    }

    if (serverPort <= 0 || serverPort >= 65535) {
        printf("Invalid port provided, %d. Please provide a port between 1 and %d", serverPort, 65535);
        return 1;
    }

    printf("%d", serverPort);
}