#ifndef CAB403ASSIGNMENT1_SERVER_H
#define CAB403ASSIGNMENT1_SERVER_H
#define SERVER

#include "../common/common.h"
#include <signal.h>

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

#endif //CAB403ASSIGNMENT1_SERVER_H
