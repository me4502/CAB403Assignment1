#pragma once

#define DEFAULT_PORT 12345
#define BACKLOG 10
#define BUFFER_SIZE 256

#include <stdbool.h>
#include <stdint.h>

typedef struct str_pair {
    char * a;
    char * b;
} StrPair;

typedef struct client_game_state {
    int remainingGuesses;
    char * guessedLetters;
    char * currentGuess;
    bool won;
} ClientGameState;

// Data packets
// Client -> Server (0-127)
#define LOGIN_PACKET 0
#define START_PACKET 1
#define GUESS_PACKET 2

// Server -> Client (128-256)
# define SEND_STATE ((1 << 7 ) | 0)

typedef struct data_packet {
    uint8_t type;
    char username[16];
    void * payload;
} DataPacket;

typedef struct login_details_payload {
    char password[16];
} LoginDetailsPayload;

typedef struct take_turn_payload {
    char guess;
} TakeTurnPayload;

// Start Game Packet

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