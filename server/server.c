#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include "../common/map.h"
#include "../common/list.h"
#include <unistd.h>
#include <sys/socket.h>

Map accounts;
List words;

int serverPort;

int main(int argc, char ** argv) {
    int sockfd, new_fd;
    struct sockaddr_in my_addr;
    signal(SIGINT, interruptHandler);

    switch(argc) {
        case 1:
            serverPort = DEFAULT_PORT;
            break;
        case 2:
            serverPort = atoi(argv[1]);
            break;
        default:
            error("Too many arguments provided")
    }

    validatePort(serverPort);

    if (loadAccounts() != 0) {
        error("Couldnt load accounts");
    }

    if (loadWords() != 0) {
        error("Couldnt load words");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("Socket could not be made");
    }

    printf("%d\n", serverPort);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[1]));
    my_addr.sin_addr.s_addr = INADDR_ANY;    // 0.0.0.0

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        error("Couldnt bind")
    }

    if (listen(sockfd, BACKLOG) == -1) {
        error("Couldnt listen");
    }

    printf("Server is listening now :) \n")

    while (1) {
        sin_size = sizeof(struct sockaddr_in) {
            if ((new_fd = accept(sockfd,
                                (struct sockaddr *) &their_addr,
                                 &sin_size)) == -1) {
                                     perror("Accepting messed up");
                                     continue;
            }
            printf("Got a connection from: %s\n", inet_ntoa(their_addr.sin_addr));
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&client_thread, &attr, TODOXXX, new_fd);
        }
    }

    return 0;
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
        char * token = strtok(line, "\t\n ");
        int tokenNumber = 0;

        char * username = NULL;
        char * password = NULL;

        while (token != NULL) {
            if (tokenNumber == 0) {
                username = malloc(strlen(token) * sizeof(char));
                copy_string(token, username);
            } else if (tokenNumber == 1) {
                password = malloc(strlen(token) * sizeof(char));
                copy_string(token, password);
            }
            token = strtok(NULL, "\t\n ");
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
                    pair->a = malloc(strlen(token) * sizeof(char));
                    copy_string(token, pair->a);
                    break;
                case 1:
                    pair->b = malloc(strlen(token) * sizeof(char));
                    copy_string(token, pair->b);
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
    if (signal == SIGINT) {
        // I put the \n at the start so it reads nicer :)
        printf("\nReceived SIGINT\nI should exit cleanly now.\n");
        finishUp();
        exit(0);
    }
}

void finishUp() {
    // TODO Handle required socket/thread closures.
    return;
}