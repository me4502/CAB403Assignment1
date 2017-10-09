#pragma once

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

void startGame();

void guessCharacter(char character);
