#include "client.h"

char * serverAddress;
int serverPort;

int main(int argc, char * argv[]) {
    switch(argc) {
        case 1:
            printf("Too few arguments provided. Usage: <program> <server IP> <server port>");
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

    if (serverPort <= 0 || serverPort >= 65535) {
        printf("Invalid port provided, %d. Please provide a port between 1 and %d", serverPort, 65535);
        return 1;
    }
    if (serverAddress == NULL) {
        printf("Please provide a server address!");
        return 1;
    }

    printf("%s:%d", serverAddress, serverPort);
}