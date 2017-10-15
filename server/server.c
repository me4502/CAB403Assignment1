#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>
#include "../common/map.h"
#include "../common/list.h"
#include "../common/common.h"
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

Map accounts;
List words;

List gameSessions;

uint16_t serverPort;
int sockfd, new_fd;

volatile int highestSession = 0;

int main(int argc, char ** argv) {
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
    pthread_t client_thread;
    signal(SIGINT, interruptHandler);
    signal(SIGHUP, interruptHandler);

    switch (argc) {
        case 1:
            serverPort = DEFAULT_PORT;
            break;
        case 2:
            serverPort = atoi(argv[1]);
            break;
        default:
            error("Too many arguments provided");
    }

    validatePort(serverPort);

    if (loadAccounts() != 0) {
        error("Failed to load accounts");
    }

    if (loadWords() != 0) {
        error("Failed to load words list");
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("Failed to create socket");
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(serverPort);
    my_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        error("Failed to bind to port");
    }

    if (listen(sockfd, BACKLOG) == -1) {
        error("Failed to listen on socket");
    }

    gameSessions = createList(8, sizeof(ServerGameState));

    printf("Server is listening on port %d \n", serverPort);

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
            perror("Failed to accept a connection");
            continue;
        }
        printf("Got a connection from: %s\n", inet_ntoa(their_addr.sin_addr));
        pthread_create(&client_thread, NULL, handleResponse, &new_fd);
    }

    return 0;
}

int _getStateIndexBySession(int session) {
    for (int i = 0; i < gameSessions->length; i++) {
        ServerGameState * serverGameState = ((ServerGameState *) getValueAt(gameSessions, i));
        if (serverGameState == NULL) {
            error("Encountered null gamestate!");
        }
        if (serverGameState->session == session) {
            return i;
        }
    }

    return -1;
}

ServerGameState * _getStateBySession(int session) {
    int index = _getStateIndexBySession(session);

    return index != -1 ? getValueAt(gameSessions, index) : NULL;
}

void formatWords(StrPair * randomWordPair, char guesses[GUESSED_LETTERS_LENGTH],
                 char formattedWords[CURRENT_GUESS_LENGTH], int * remainingSpots) {
    (* remainingSpots) = 0;
    int index = 0;
    for (int i = 0; i < strlen(randomWordPair->a); i++) {
        if (strchr(guesses, randomWordPair->a[i])) {
            formattedWords[index ++] = randomWordPair->a[i];
        } else {
            formattedWords[index ++] = '_';
            (* remainingSpots) ++;
        }
        formattedWords[index ++] = ' ';
    }
    formattedWords[index ++] = ' ';
    formattedWords[index ++] = ' ';
    for (int i = 0; i < strlen(randomWordPair->b); i++) {
        if (strchr(guesses, randomWordPair->b[i])) {
            formattedWords[index ++] = randomWordPair->b[i];
        } else {
            formattedWords[index ++] = '_';
            (* remainingSpots) ++;
        }
        formattedWords[index ++] = ' ';
    }
    formattedWords[index] = '\0';
}

void * handleResponse(void * socket_id) {
    int sock = *(int *) socket_id;
    DataPacket * inputPacket = malloc(sizeof(DataPacket));

    while (recv(sock, inputPacket, sizeof(DataPacket), 0) > 0) {
        switch (inputPacket->type) {
            case LOGIN_PACKET: {
                LoginDetailsPayload *detailsPayload = malloc(sizeof(LoginDetailsPayload));
                recv(sock, detailsPayload, sizeof(LoginDetailsPayload), 0);

                char username[16], password[16];
                strcpy(username, detailsPayload->username);
                strcpy(password, detailsPayload->password);

                bool response = true;
                if (containsEntry(accounts, username)) {
                    if (strcmp(getValue(accounts, username), password) == 0) {
                        response = true;
                    }
                }

                LoginResponsePayload loginResponse;
                loginResponse.success = response;

                DataPacket packet;
                packet.type = LOGIN_RESPONSE_PACKET;
                packet.session = response ? (highestSession++) : -1;

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &loginResponse, sizeof(LoginResponsePayload), 0);
                free(detailsPayload);
                break;
            }
            case START_PACKET: {
                StrPair *randomWordPair = (StrPair *) getValueAt(words, rand() % words->length);

                ServerGameState serverState;
                serverState.session = inputPacket->session;
                serverState.wordPair = randomWordPair;
                strcpy(serverState.guessedLetters, "");
                serverState.guessesLeft = (int) min(strlen(randomWordPair->a) + strlen(randomWordPair->b) + 10, 26);

                add(gameSessions, &serverState);

                ClientGameState state;

                state.remainingGuesses = serverState.guessesLeft;
                strcpy(state.guessedLetters, serverState.guessedLetters);
                int remaining = 0;
                formatWords(randomWordPair, state.guessedLetters, state.currentGuess, &remaining);
                state.won = false;

                DataPacket packet;
                packet.type = STATE_RESPONSE_PACKET;
                packet.session = inputPacket->session;

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &state, sizeof(ClientGameState), 0);
                break;
            }
            case GUESS_PACKET: {
                TakeTurnPayload *takeTurnPayload = malloc(sizeof(TakeTurnPayload));
                recv(sock, takeTurnPayload, sizeof(TakeTurnPayload), 0);

                ServerGameState *serverState = _getStateBySession(inputPacket->session);
                if (serverState == NULL) {
                    printf("Got invalid session %d", inputPacket->session);
                    free(takeTurnPayload);
                    free(inputPacket);
                    break;
                }
                if (takeTurnPayload->guess < 'a' || takeTurnPayload->guess > 'z'
                    || strchr(serverState->guessedLetters, takeTurnPayload->guess)) {
                    DataPacket packet;
                    packet.type = INVALID_GUESS_PACKET;
                    packet.session = inputPacket->session;

                    send(sock, &packet, sizeof(DataPacket), 0);
                    free(takeTurnPayload);
                    break;
                }
                serverState->guessesLeft--;
                size_t currentGuessNumbers = strlen(serverState->guessedLetters);
                serverState->guessedLetters[currentGuessNumbers] = takeTurnPayload->guess;
                serverState->guessedLetters[currentGuessNumbers + 1] = '\0';

                ClientGameState state;

                state.remainingGuesses = serverState->guessesLeft;
                strcpy(state.guessedLetters, serverState->guessedLetters);
                int remaining;
                formatWords(serverState->wordPair, state.guessedLetters, state.currentGuess, &remaining);
                state.won = remaining == 0;

                DataPacket packet;
                packet.type = STATE_RESPONSE_PACKET;
                packet.session = inputPacket->session;


                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &state, sizeof(ClientGameState), 0);

                free(takeTurnPayload);
                break;
            }
            case CLOSE_CLIENT_PACKET: {
                if (inputPacket->session != -1) {
                    // Clear out their old session.
                    removeAt(gameSessions, _getStateIndexBySession(inputPacket->session));
                    return NULL;
                }
            }
            default:
                perror("Unknown packet type");
                break;
        }
    }

    return NULL;
}

int loadAccounts() {
    accounts = createMap(16);

    FILE * auth_file_handle = fopen("Authentication.txt", "r");
    if (auth_file_handle == NULL) {
        printf("Failed to open Authentication.txt! Does it exist?\n");
        return 1;
    }

    size_t len = 0;
    char * line = NULL;
    int lineNum = -1;

    while ((getline(&line, &len, auth_file_handle)) != -1) {
        lineNum++;

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
                strcpy(username, token);
            } else if (tokenNumber == 1) {
                password = malloc(strlen(token) * sizeof(char));
                strcpy(password, token);
            }
            token = strtok(NULL, "\t\n ");
            tokenNumber++;
        }

        if (username == NULL || password == NULL) {
            printf("Invalid username and password on line %d.\n", lineNum + 1);
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
        printf("Failed to open hangman_text.txt! Does it exist?\n");
        return 1;
    }

    size_t len = 0;
    char * line = malloc(16 * sizeof(char));
    int lineNum = -1;

    while (getline(&line, &len, hangman_file_handle) != -1) {
        lineNum++;

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
                    strcpy(pair->a, token);
                    break;
                case 1:
                    pair->b = malloc(strlen(token) * sizeof(char));
                    strcpy(pair->b, token);
                    break;
                default:
                    printf("Malformed word pair on line: %d.\n", lineNum + 1);
                    fclose(hangman_file_handle);
                    return 1;
            }
            token = strtok(NULL, ",\n");
            tokenNumber++;
        }

        if (tokenNumber != 2) {
            printf("Malformed word pair on line: %d.\n", lineNum + 1);
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
    if (signal == SIGINT || signal == SIGHUP) {
        // I put the \n at the start so it reads nicer :)
        printf("\nReceived interrupt\nI should exit cleanly now.\n");
        finishUp();
        exit(0);
    }
}

void finishUp() {
    // TODO Handle required socket/thread closures.
    freeMap(accounts);
    freeList(words);
    shutdown(sockfd, SHUT_RDWR);
}