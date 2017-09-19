#include "server.h"

Map accounts;

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

    loadAccounts();

    printf("%d\n", serverPort);

    signal(SIGINT, interruptHandler);
}

int loadAccounts() {
    accounts = createMap(16);

    FILE * auth_file_handle = fopen("Authentication.txt", "r");
    if (auth_file_handle == NULL) {
        printf("Failed to open Authentication.txt! Does it exist?");
        return 1;
    }

    size_t len = 0;
    char * line = NULL;
    int lineNum = -1;

    while ((getline(&line, &len, auth_file_handle)) != -1) {
        lineNum ++;

        if (lineNum == 0) {
            continue;
        }
        char * token = strtok(line, "\t ");
        int tokenNumber = 0;

        char * username = NULL;
        char * password = NULL;

        while (token != NULL) {
            if (tokenNumber == 0) {
                username = token;
            } else if (tokenNumber == 1) {
                password = token;
            }
            token = strtok(NULL, "\t ");
            tokenNumber ++;
        }

        if (username == NULL || password == NULL) {
            printf("Invalid username and password on line %d.", lineNum + 1);
            return 1;
        }
        putEntry(accounts, username, password);
    }

    fclose(auth_file_handle);

    return 0;
}

void interruptHandler(int signal) {
    // TODO Handle required socket/thread closures.
}