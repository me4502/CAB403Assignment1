#ifndef CAB403ASSIGNMENT1_CLIENT_H
#define CAB403ASSIGNMENT1_CLIENT_H
#define CLIENT

#include "../common/common.h"

typedef enum screen_type {
    LOGIN_SCREEN,
    MENU_SCREEN,
    GAME_OVER_SCREEN,
    LEADERBOARD_SCREEN
} ScreenType;

void setupScreen();

void cleanupScreen();

#endif //CAB403ASSIGNMENT1_CLIENT_H
