#ifndef CAB403ASSIGNMENT1_SERVER_H
#define CAB403ASSIGNMENT1_SERVER_H
#define SERVER

#include "../common/common.h"

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

#endif //CAB403ASSIGNMENT1_SERVER_H
