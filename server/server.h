#pragma once

#include "../common/common.h"

typedef struct server_game_state {
    int session;
    StrPair * wordPair;
    int guessesLeft;
    char * guessedLetters;
} ServerGameState;

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
 * Ensures everything is wrapped up and ready for a clean exit
 */
void finishUp();

void * handleResponse(void * socket_id);