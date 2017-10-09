#ifndef CAB403ASSIGNMENT1_COMMON_H
#define CAB403ASSIGNMENT1_COMMON_H

#define DEFAULT_PORT 12345
#define BACKLOG 10
#define BUFFER_SIZE 256

#include <stdbool.h>

typedef struct str_pair {
    char * a;
    char * b;
} StrPair;

typedef struct client_game_state {
    char username[16];
    int remainingGuesses;
    char * guessedLetters;
    char * currentGuess;
    bool won;
} ClientGameState;

/**
 * Validates that the given port is valid.
 *
 * @param port The port to validate
 * @return Return code. 0 = success, 1 = failure
 */
int validatePort(int port);

/**
 * Copy a string from one char * to another.
 *
 * @param from The from string
 * @param to The to string
 */
void copy_string(char * from, char * to);

/**
 * Prints msg then dies
 */
void error(char *msg);

#endif //CAB403ASSIGNMENT1_COMMON_H
