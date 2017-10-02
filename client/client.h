#ifndef CAB403ASSIGNMENT1_CLIENT_H
#define CAB403ASSIGNMENT1_CLIENT_H
#define CLIENT

#include "../common/common.h"

#include <stdbool.h>

typedef enum screen_type {
    LOGIN_SCREEN,
    MENU_SCREEN,
    GAME_OVER_SCREEN,
    LEADERBOARD_SCREEN
} ScreenType;

void drawWelcomeText();

void drawScreen(ScreenType screenType);

bool authenticateUser(char username[], char password[]);

#endif //CAB403ASSIGNMENT1_CLIENT_H
