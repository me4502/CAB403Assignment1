#ifndef __APPLE__
#define _GNU_SOURCE
#endif

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

Map scores;

List currentSessions;
List gameSessions;

int _DIENOW = 0;
int _DEAD = 0;


// Mutexes are different on macOS for some reason
#ifdef __APPLE__
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
pthread_mutex_t scores_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t scores_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

pthread_cond_t got_request = PTHREAD_COND_INITIALIZER;
int num_requests = 0;
struct request * requests = NULL;
struct request * last_request = NULL;

/**
 * Add a request to the requests list
 *
 * Creates a request structure, adds to the list, and increases number of pending requests by one.
 *
 * @param socket_id The socket ID
 * @param p_mutex The mutex linked list
 * @param p_cond_var The thread condition
 */
void add_request(int socket_id, pthread_mutex_t * p_mutex, pthread_cond_t * p_cond_var) {
    // Create the new request
    struct request * newRequest = (struct request *) malloc(sizeof(struct request));
    if (!newRequest) {
        error("Out of memory");
        return;
    }
    newRequest->socket_id = socket_id;
    newRequest->next = NULL;

    pthread_mutex_lock(p_mutex);

    // Add the new request to the list
    if (num_requests == 0) {
        requests = newRequest;
        last_request = newRequest;
    } else {
        last_request->next = newRequest;
        last_request = newRequest;
    }

    num_requests++;

    pthread_mutex_unlock(p_mutex);
    pthread_cond_signal(p_cond_var);
}

/**
 * Gets the first pending request from the requests list
 * removing it from the list.
 *
 * Creates a request structure, adds to the list, and
 * increases number of pending requests by one.
 *
 * <p>
 *      The returned memory must be freed.
 * </p>
 *
 * @param p_mutex The linked list mutex
 * @return The removed request, or NULL
 */
struct request * get_request(pthread_mutex_t * p_mutex) {
    struct request * a_request;

    pthread_mutex_lock(p_mutex);

    if (num_requests > 0) {
        a_request = requests;
        requests = a_request->next;
        if (requests == NULL) {
            last_request = NULL;
        }

        num_requests--;
    } else {
        a_request = NULL;
    }

    pthread_mutex_unlock(p_mutex);

    return a_request;
}

void * handleResponseLoop(void * data) {
    struct request * a_request;
    int thread_id = *((int *) data);

    pthread_mutex_lock(&request_mutex);

    while (!_DIENOW) {
        if (num_requests > 0) {
            a_request = get_request(&request_mutex);
            if (a_request) {
                pthread_mutex_unlock(&request_mutex);
                handleResponse(a_request, thread_id);
                free(a_request);
                pthread_mutex_lock(&request_mutex);
            }
        } else {
            pthread_cond_wait(&got_request, &request_mutex);
        }
    }
    _DEAD++;
}

uint16_t serverPort;
int sockfd, new_fd;

volatile int highestSession = 0;

int main(int argc, char ** argv) {
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
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
            return -1;
    }

    validatePort(serverPort);

    if (loadAccounts() != 0) {
        error("Failed to load accounts");
        return -1;
    }

    if (loadWords() != 0) {
        error("Failed to load words list");
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("Failed to create socket");
        return -1;
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(serverPort);
    my_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0

    if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1) {
        error("Failed to bind to port");
        return -1;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        error("Failed to listen on socket");
        return -1;
    }

    currentSessions = createList(4, sizeof(SessionStore));
    gameSessions = createList(4, sizeof(ServerGameState));
    scores = createMap(4);

    printf("Server is listening on port %d \n", serverPort);


    int threadIds[BACKLOG];
    pthread_t p_threads[BACKLOG];

    for (int i = 0; i < BACKLOG; i++) {
        threadIds[i] = i;
        pthread_create(&p_threads[i], NULL, handleResponseLoop, (void *) &threadIds[i]);
    }

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
            perror("Failed to accept a connection");
            continue;
        }
        printf("Got a connection from: %s\n", inet_ntoa(their_addr.sin_addr));
        add_request(new_fd, &request_mutex, &got_request);
    }

    return 0;
}

int _getStateIndexBySession(int session) {
    for (int i = 0; i < gameSessions->length; i++) {
        ServerGameState * serverGameState = ((ServerGameState *) getValueAt(gameSessions, i));
        if (serverGameState == NULL) {
            error("Encountered null gamestate!");
            return -1;
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
    (*remainingSpots) = 0;
    int index = 0;
    for (int i = 0; i < strlen(randomWordPair->a); i++) {
        if (strchr(guesses, randomWordPair->a[i])) {
            formattedWords[index++] = randomWordPair->a[i];
        } else {
            formattedWords[index++] = '_';
            (*remainingSpots)++;
        }
        formattedWords[index++] = ' ';
    }
    formattedWords[index++] = ' ';
    formattedWords[index++] = ' ';
    for (int i = 0; i < strlen(randomWordPair->b); i++) {
        if (strchr(guesses, randomWordPair->b[i])) {
            formattedWords[index++] = randomWordPair->b[i];
        } else {
            formattedWords[index++] = '_';
            (*remainingSpots)++;
        }
        formattedWords[index++] = ' ';
    }
    formattedWords[index] = '\0';
}

void * handleResponse(struct request * a_request, int thread_id) {
    int sock = a_request->socket_id;
    DataPacket inputPacket;

    while (recv(sock, &inputPacket, sizeof(DataPacket), 0) > 0) {
        SessionStore * currentSession = NULL;
        if (inputPacket.session != -1) {
            for (int i = 0; i < currentSessions->length; i++) {
                SessionStore * sessionStore = getValueAt(currentSessions, i);
                if (sessionStore->session == inputPacket.session) {
                    currentSession = sessionStore;
                    break;
                }
            }
        }

        switch (inputPacket.type) {
            case LOGIN_PACKET: {
                LoginDetailsPayload detailsPayload;
                recv(sock, &detailsPayload, sizeof(LoginDetailsPayload), 0);

                char username[USERNAME_MAX_LENGTH], password[PASSWORD_MAX_LENGTH];
                strcpy(username, detailsPayload.username);
                strcpy(password, detailsPayload.password);

                bool response = false;
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

                if (response) {
                    SessionStore * store = malloc(sizeof(SessionStore));
                    strcpy(store->username, username);
                    store->session = packet.session;
                    add(currentSessions, store);
                }

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &loginResponse, sizeof(LoginResponsePayload), 0);
                break;
            }
            case START_PACKET: {
                if (currentSession == NULL) {
                    printf("Non-logged in user tried to start a game!");
                    break;
                }
                StrPair * randomWordPair = (StrPair *) getValueAt(words, (int) (random() % words->length));

                ServerGameState serverState;
                serverState.session = inputPacket.session;
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
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &state, sizeof(ClientGameState), 0);
                break;
            }
            case GUESS_PACKET: {
                if (currentSession == NULL) {
                    printf("Non-logged in user tried to guess!");
                    break;
                }
                TakeTurnPayload takeTurnPayload;
                recv(sock, &takeTurnPayload, sizeof(TakeTurnPayload), 0);

                ServerGameState * serverState = _getStateBySession(inputPacket.session);
                if (serverState == NULL) {
                    printf("Got invalid session %d", inputPacket.session);
                    break;
                }
                if (takeTurnPayload.guess < 'a' || takeTurnPayload.guess > 'z'
                    || strchr(serverState->guessedLetters, takeTurnPayload.guess)
                    || serverState->guessesLeft <= 0) {
                    DataPacket packet;
                    packet.type = INVALID_GUESS_PACKET;
                    packet.session = inputPacket.session;

                    send(sock, &packet, sizeof(DataPacket), 0);
                    break;
                }
                serverState->guessesLeft--;
                size_t currentGuessNumbers = strlen(serverState->guessedLetters);
                serverState->guessedLetters[currentGuessNumbers] = takeTurnPayload.guess;
                serverState->guessedLetters[currentGuessNumbers + 1] = '\0';

                ClientGameState state;

                state.remainingGuesses = serverState->guessesLeft;
                strcpy(state.guessedLetters, serverState->guessedLetters);
                int remaining;
                formatWords(serverState->wordPair, state.guessedLetters, state.currentGuess, &remaining);
                state.won = remaining == 0;

                if (state.won || serverState->guessesLeft <= 0) {
                    LeaderboardEntry * entry = getScoreForPlayer(currentSession->username);
                    entry->games++;
                    strcpy(entry->username, currentSession->username);
                    if (state.won) {
                        entry->wins++;
                    }
                }

                DataPacket packet;
                packet.type = STATE_RESPONSE_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);
                send(sock, &state, sizeof(ClientGameState), 0);
                break;
            }
            case LEADERBOARD_PACKET: {
                if (currentSession == NULL) {
                    printf("Non-logged in user tried to request leaderboard!");
                    break;
                }
                DataPacket packet;
                packet.type = START_LEADERBOARD_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);

                int length;
                pthread_mutex_lock(&scores_mutex);
                LeaderboardEntry ** leaderboardArray = (LeaderboardEntry **) getValues(scores, sizeof(LeaderboardEntry),
                                                                                       &length);
                pthread_mutex_unlock(&scores_mutex);

                if (length > 1) {
                    for (int i = length - 1; i > 0; i--) {
                        for (int j = 0; j < i; ++j) {
                            LeaderboardEntry * a = leaderboardArray[j];
                            LeaderboardEntry * b = leaderboardArray[j + 1];
                            if (a->wins > b->wins
                                || (a->wins == b->wins && (a->wins / (float) a->games) > (b->wins / (float) b->games))
                                || strcmp(a->username, b->username) > 0) {
                                leaderboardArray[j + 1] = a;
                                leaderboardArray[j] = b;
                            }
                        }
                    }
                }

                for (int i = 0; i < length; i++) {
                    LeaderboardEntry * leaderboardEntry = leaderboardArray[i];

                    packet.type = ENTRY_LEADERBOARD_PACKET;
                    packet.session = inputPacket.session;

                    send(sock, &packet, sizeof(DataPacket), 0);
                    send(sock, leaderboardEntry, sizeof(LeaderboardEntry), 0);
                }

                packet.type = END_LEADERBOARD_PACKET;
                packet.session = inputPacket.session;

                send(sock, &packet, sizeof(DataPacket), 0);
                break;
            }
            case CLOSE_CLIENT_PACKET: {
                if (inputPacket.session != -1) {
                    // Clear out their old session.
                    removeAt(gameSessions, _getStateIndexBySession(inputPacket.session));
                    for (int i = 0; i < currentSessions->length; i++) {
                        SessionStore * sessionStore = getValueAt(currentSessions, i);
                        if (sessionStore->session == inputPacket.session) {
                            removeAt(currentSessions, i);
                            break;
                        }
                    }
                    return NULL;
                }
                break;
            }
            default:
                perror("Unknown packet type");
                break;
        }
    }

    return NULL;
}

LeaderboardEntry * getScoreForPlayer(char username[USERNAME_MAX_LENGTH]) {
    pthread_mutex_lock(&scores_mutex);
    LeaderboardEntry * entry = getValue(scores, username);
    pthread_mutex_unlock(&scores_mutex);
    if (entry == NULL) {
        entry = malloc(sizeof(LeaderboardEntry));
        entry->games = 0;
        entry->wins = 0;
        pthread_mutex_lock(&scores_mutex);
        putEntry(scores, username, entry);
        pthread_mutex_unlock(&scores_mutex);
    }

    return entry;
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
        char * token = strtok(line, "\t\n\r ");
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
            token = strtok(NULL, "\t\n\r ");
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
    char * line = NULL;
    int lineNum = -1;

    while (getline(&line, &len, hangman_file_handle) != -1) {
        lineNum++;

        if (lineNum == 0) {
            continue;
        }
        char * token = strtok(line, ",\n\r");
        int tokenNumber = 0;

        StrPair * pair = malloc(sizeof(StrPair));

        while (token != NULL) {
            switch (tokenNumber) {
                case 0:
                    pair->b = malloc(strlen(token) * sizeof(char));
                    strcpy(pair->b, token);
                    break;
                case 1:
                    pair->a = malloc(strlen(token) * sizeof(char));
                    strcpy(pair->a, token);
                    break;
                default:
                    printf("Malformed word pair on line: %d.\n", lineNum + 1);
                    fclose(hangman_file_handle);
                    return 1;
            }
            token = strtok(NULL, ",\n\r");
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
    _DIENOW = 1;
    while (_DEAD < BACKLOG) {
        sleep(10);
    }
    freeMap(accounts);
    freeList(words);
    freeList(gameSessions);
    freeList(currentSessions);
    freeMap(scores);
    shutdown(sockfd, SHUT_RDWR);
}