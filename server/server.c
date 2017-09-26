#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../common/map.h"
#include "../common/list.h"

Map accounts;
List words;

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

    if (loadAccounts() != 0) {
        return 1;
    }
    if (loadWords() != 0) {
        return 1;
    }

    for (int i = 0; i < accounts->length; i++) {
        if (accounts->entries[i] == NULL) {
            continue;
        }
        printf("%s=%s\n", (char *) accounts->entries[i]->key, (char *) accounts->entries[i]->value);
    }

    for (int i = 0; i < words->length; i++) {
        StrPair * strPair = (StrPair *) getValueAt(words, i);
        printf("%s %s\n", strPair->a, strPair->b);
    }

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
            fclose(auth_file_handle);
            return 1;
        }
        putEntry(accounts, username, password);
    }

    fclose(auth_file_handle);

    return 0;
}

int loadWords() {
    words = createList(16, sizeof(StrPair));

    FILE * hangman_file_handle = fopen("hangman_text.txt", "r");
    if (hangman_file_handle == NULL) {
        printf("Failed to open hangman_text.txt! Does it exist?");
        return 1;
    }

    size_t len = 0;
    char * line = malloc(16 * sizeof(char));
    int lineNum = -1;

    while (getline(&line, &len, hangman_file_handle) != -1) {
        lineNum ++;

        if (lineNum == 0) {
            continue;
        }
        char * token = strtok(line, ",\n");
        int tokenNumber = 0;

        StrPair * pair = malloc(sizeof(StrPair));

        while (token != NULL) {
            switch (tokenNumber) {
                case 0:
                    pair->a = token;
                    break;
                case 1:
                    pair->b = token;
                    break;
                default:
                    printf("Malformed word pair on line: %d.", lineNum + 1);
                    fclose(hangman_file_handle);
                    return 1;
            }
            token = strtok(NULL, ",\n");
            tokenNumber ++;
        }

        if (tokenNumber != 2) {
            printf("Malformed word pair on line: %d.", lineNum + 1);
            fclose(hangman_file_handle);
            return 1;
        }

        add(words, pair);
    }

    fclose(hangman_file_handle);
    free(line);

    return 0;
}

void interruptHandler(int signal) {
    // TODO Handle required socket/thread closures.
}