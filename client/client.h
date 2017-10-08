#ifndef CAB403ASSIGNMENT1_CLIENT_H
#define CAB403ASSIGNMENT1_CLIENT_H
#define CLIENT

#include "../common/common.h"

#include <stdbool.h>

typedef enum screen_type {
    LOGIN_SCREEN,
    MENU_SCREEN,
    GAME_OVER_SCREEN,
    WIN_SCREEN,
    LEADERBOARD_SCREEN,
    GAME_SCREEN
} ScreenType;

void drawWelcomeText();

void drawScreen(ScreenType screenType);

bool authenticateUser(char username[], char password[]);

ClientGameState startGame();

ClientGameState guessCharacter(ClientGameState currentState, char character);

#endif //CAB403ASSIGNMENT1_CLIENT_H
