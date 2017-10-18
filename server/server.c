#define _GNU_SOURCE
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


/* global mutex for our program. assignment initializes it. */
/* note that we use a RECURSIVE mutex, since a handler      */
/* thread might try to lock it twice consecutively.         */
pthread_mutex_t request_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/* global condition variable for our program. assignment initializes it. */
pthread_cond_t  got_request   = PTHREAD_COND_INITIALIZER;
int num_requests = 0; // Pending requests
struct request* requests = NULL;     /* head of linked list of requests. */
struct request* last_request = NULL; /* pointer to last request.         */

/*
 * function add_request(): add a request to the requests list
 * algorithm: creates a request structure, adds to the list, and
 *            increases number of pending requests by one.
 * input:     request number, linked list mutex.
 * output:    none.
 */
void add_request(int socket_id, pthread_mutex_t* p_mutex, pthread_cond_t*  p_cond_var) {
    int rc;                         /* return code of pthreads functions.  */
    struct request* a_request;      /* pointer to newly added request.     */

    /* create structure with new request */
    a_request = (struct request*)malloc(sizeof(struct request));
    if (!a_request) { /* malloc failed?? */
        fprintf(stderr, "add_request: out of memory\n");
        exit(1);
    }
    a_request->socket_id = socket_id;
    a_request->next = NULL;

    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(p_mutex);

    /* add new request to the end of the list, updating list */
    /* pointers as required */
    if (num_requests == 0) { /* special case - list is empty */
        requests = a_request;
        last_request = a_request;
    }
    else {
        last_request->next = a_request;
        last_request = a_request;
    }

    /* increase total number of pending requests by one. */
    num_requests++;

    /* unlock mutex */
    rc = pthread_mutex_unlock(p_mutex);

    /* signal the condition variable - there's a new request to handle */
    rc = pthread_cond_signal(p_cond_var);
}

/*
 * function get_request(): gets the first pending request from the requests list
 *                         removing it from the list.
 * algorithm: creates a request structure, adds to the list, and
 *            increases number of pending requests by one.
 * input:     request number, linked list mutex.
 * output:    pointer to the removed request, or NULL if none.
 * memory:    the returned request need to be freed by the caller.
 */
struct request* get_request(pthread_mutex_t* p_mutex) {
    int rc;                         /* return code of pthreads functions.  */
    struct request* a_request;      /* pointer to request.                 */

    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(p_mutex);

    if (num_requests > 0) {
        a_request = requests;
        requests = a_request->next;
        if (requests == NULL) { /* this was the last request on the list */
            last_request = NULL;
        }
        /* decrease the total number of pending requests */
        num_requests--;
    }
    else { /* requests list is empty */
        a_request = NULL;
    }

    /* unlock mutex */
    rc = pthread_mutex_unlock(p_mutex);

    /* return the request to the caller. */
    return a_request;
}

void* handleResponseLoop(void* data) {
    int rc;
    struct request* a_request;
    int thread_id = *((int*) data);

    rc = pthread_mutex_lock(&request_mutex);

    while (1) {
        if (num_requests > 0) {
            a_request = get_request(&request_mutex);
            if (a_request) {
                rc = pthread_mutex_unlock(&request_mutex);
                handleResponse(a_request, thread_id);
                free(a_request);
                rc = pthread_mutex_lock(&request_mutex);
            }
        } else {
            rc = pthread_cond_wait(&got_request, &request_mutex);
        }
    }
}

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

    currentSessions = createList(4, sizeof(SessionStore));
    gameSessions = createList(4, sizeof(ServerGameState));
    scores = createMap(4);

    printf("Server is listening on port %d \n", serverPort);


    int        thr_id[BACKLOG];      /* thread IDs            */
    pthread_t  p_threads[BACKLOG];   /* thread's structures   */

    /* create the request-handling threads */
    for (int i = 0; i < BACKLOG; i++) {
        thr_id[i] = i;
        pthread_create(&p_threads[i], NULL, handleResponseLoop, (void*)&thr_id[i]);
    }

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size)) == -1) {
            perror("Failed to accept a connection");
            continue;
        }
        printf("Got a connection from: %s\n", inet_ntoa(their_addr.sin_addr));
        add_request(&new_fd, &request_mutex, &got_request);
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

void * handleResponse(struct request* a_request, int thread_id) {
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
                StrPair *randomWordPair = (StrPair *) getValueAt(words, (int) (random() % words->length));

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

                ServerGameState *serverState = _getStateBySession(inputPacket.session);
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
                LeaderboardEntry ** leaderboardArray = (LeaderboardEntry **) getValues(scores, sizeof(LeaderboardEntry), &length);
                // TODO Sort
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
    LeaderboardEntry * entry = getValue(scores, username);
    if (entry == NULL) {
        entry = malloc(sizeof(LeaderboardEntry));
        entry->games = 0;
        entry->wins = 0;
        putEntry(scores, username, entry);
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
    // TODO Handle required socket/thread closures.
    freeMap(accounts);
    freeList(words);
    freeList(gameSessions);
    freeList(currentSessions);
    freeMap(scores);
    shutdown(sockfd, SHUT_RDWR);
}