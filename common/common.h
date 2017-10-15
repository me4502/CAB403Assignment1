#pragma once

#define DEFAULT_PORT 12345
#define BACKLOG 10

// Length definitions
#define USERNAME_MAX_LENGTH 16
#define PASSWORD_MAX_LENGTH 16
#define GUESSED_LETTERS_LENGTH 26
#define CURRENT_GUESS_LENGTH 32

#define min(a,b) ((a) > (b) ? (b) : (a))

#include <stdbool.h>
#include <stdint.h>

typedef struct str_pair {
    char * a;
    char * b;
} StrPair;

typedef struct client_game_state {
    int remainingGuesses;
    char guessedLetters[GUESSED_LETTERS_LENGTH];
    char currentGuess[CURRENT_GUESS_LENGTH];
    bool won;
} ClientGameState;

typedef struct leaderboard_entry {
    char username[USERNAME_MAX_LENGTH];
    int wins;
    int games;
} LeaderboardEntry;

// Data packets
// Client -> Server (0-127)
#define LOGIN_PACKET 0
#define START_PACKET 1
#define GUESS_PACKET 2
#define CLOSE_CLIENT_PACKET 3
#define LEADERBOARD_PACKET 4

// Server -> Client (128-256)
#define LOGIN_RESPONSE_PACKET ((1 << 7 ) | 0)
#define STATE_RESPONSE_PACKET ((1 << 7 ) | 1)
#define CLOSE_SERVER_PACKET ((1 << 7) | 2)
#define INVALID_GUESS_PACKET ((1 << 7) | 3)
#define START_LEADERBOARD_PACKET ((1 << 7) | 4)
#define ENTRY_LEADERBOARD_PACKET ((1 << 7) | 5)
#define END_LEADERBOARD_PACKET ((1 << 7) | 6)

typedef struct data_packet {
    uint8_t type;
    int session;
} DataPacket;

typedef struct login_details_payload {
    char username[USERNAME_MAX_LENGTH];
    char password[PASSWORD_MAX_LENGTH];
} LoginDetailsPayload;

typedef struct take_turn_payload {
    char guess;
} TakeTurnPayload;

typedef struct login_response_payload {
    bool success;
} LoginResponsePayload;

// Start Game Packet

/**
 * Validates that the given port is valid.
 *
 * @param port The port to validate
 * @return Return code. 0 = success, 1 = failure
 */
int validatePort(int port);

/**
 * Prints msg then dies
 */
void error(char * msg);