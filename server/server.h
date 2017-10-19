#pragma once

#include "../common/common.h"

typedef struct session_store {
    int session;
    char username[USERNAME_MAX_LENGTH];
} SessionStore;

typedef struct server_game_state {
    int session;
    StrPair * wordPair;
    int guessesLeft;
    char guessedLetters[GUESSED_LETTERS_LENGTH];
} ServerGameState;

// format of a single request.
struct request {
    int socket_id;
    struct request * next;
};


/**
 * SIGINT Handler.
 *
 * @param signal The signal received
 */
void interruptHandler(int signal);

/**
 * Loads the accounts from file.
 *
 * @return 1 if an error occurred
 */
int loadAccounts();

/**
 * Loads the word pairs from file.
 *
 * @return 1 if an error occured
 */
int loadWords();

/**
 * Gets the leaderboard entry for a player.
 * Will create if it doesn't exist.
 *
 * @param username The username
 * @return The leaderboard entry
 */
LeaderboardEntry * getScoreForPlayer(char username[USERNAME_MAX_LENGTH]);

/**
 * Ensures everything is wrapped up and ready for a clean exit
 */
void finishUp();

void * handleResponse(struct request * a_request, int thread_id);
