#ifndef CAB403ASSIGNMENT1_COMMON_H
#define CAB403ASSIGNMENT1_COMMON_H

#define DEFAULT_PORT 12345

typedef struct str_pair {
    char * a;
    char * b;
} StrPair;

/**
 * Validates that the given port is valid.
 *
 * @param port The port to validate
 * @return Return code. 0 = success, 1 = failure
 */
int validatePort(int port);

#endif //CAB403ASSIGNMENT1_COMMON_H
