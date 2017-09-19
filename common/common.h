#ifndef CAB403ASSIGNMENT1_COMMON_H
#define CAB403ASSIGNMENT1_COMMON_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <memory.h>
#include "map.h"

#define DEFAULT_PORT 12345

/**
 * Validates that the given port is valid.
 *
 * @param port The port to validate
 * @return Return code. 0 = success, 1 = failure
 */
int validatePort(int port);

#endif //CAB403ASSIGNMENT1_COMMON_H
