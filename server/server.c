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

    validatePort(serverPort);

    printf("%d", serverPort);

    signal(SIGINT, interruptHandler);
    signal(SIGQUIT, interruptHandler);
}

void interruptHandler(int signal) {
    // TODO Handle required socket/thread closures.
}