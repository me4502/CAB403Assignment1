#include "client.h"

#include <stdio.h>
#include <stdlib.h>

char * serverAddress;
int serverPort;

// Maximum length of 16 for usernames and passwords.
// None are over this so therefore if more is entered it doesn't matter as it's wrong anyway.
char username[16];

int main(int argc, char ** argv) {
    switch(argc) {
        case 1:
            printf("Too few arguments provided. Usage: %s <server IP> <server port>\n", argv[0]);
            return 1;
        case 2:
            serverAddress = argv[1];
            serverPort = DEFAULT_PORT;
            break;
        case 3:
            serverAddress = argv[1];
            serverPort = atoi(argv[2]);
            break;
        default:
            printf("Too many arguments provided!\n");
            return 1;
    }

    // Validate that the given port is a valid port
    validatePort(serverPort);

    if (serverAddress == NULL) {
        printf("Please provide a server address!\n");
        return 1;
    }

    // Draw the initial welcome text
    drawWelcomeText();

    drawScreen(LOGIN_SCREEN);
}

void drawWelcomeText() {
    for (int i = 0; i < 44; i++) {
        printf("=");
    }
    printf("\n\n\nWelcome to the Online Hangman Gaming System\n\n\n");
    for (int i = 0; i < 44; i++) {
        printf("=");
    }
    printf("\n");
}

void _drawLoginScreen() {
    printf("\n\n\nYou are required to logon with your registered Username and Password\n\n");
    printf("Please enter your username -> ");
    scanf("%s", username);
    printf("Please enter your password -> ");
    char password[16];
    scanf("%s", password);
    printf("\n");

    if (authenticateUser(username, password)) {
        drawScreen(MENU_SCREEN);
    } else {
        printf("You entered either an incorrect username or password - disconnecting");
        exit(1);
    }
}

void _drawMenuScreen() {
    int status = 0;
    int choice = 0;
    while (status == 0 || choice < 1 || choice > 3) {
        printf("\nWelcome to the Hangman Gaming System\n\n\n\n");
        printf("Please enter a selection\n");
        printf("<1> Play Hangman\n");
        printf("<2> Show Leaderboard\n");
        printf("<3> Quit\n\n");
        printf("Selection option 1-3 -> ");

        status = scanf( "%d", &choice);
        if (status == 0) {
            // If the input was invalid, clear out the input buffer.
            int c;
            while((c = getchar()) != '\n' && c != EOF);
        }
    }

    switch (choice) {
        case 1:
            drawScreen(GAME_SCREEN);
            break;
        case 2:
            drawScreen(LEADERBOARD_SCREEN);
            break;
        case 3:
            return;
        default:
            drawScreen(MENU_SCREEN);
            break;
    }
}

void _drawGameOverScreen() {
    printf("\nGame over\n\n\n");
    printf("Bad luck %s! You have ran out of guesses. The Hangman got you!\n\n\n", username);
    drawScreen(MENU_SCREEN);
}

void _drawLeaderboardScreen() {
    if (true) { // TODO
        printf("\n");
        for (int i = 0; i < 77; i++) {
            printf("=");
        }
        printf("\n\nThere is no information currently stored in the Leader Board. Try again later\n\n");
        for (int i = 0; i < 77; i++) {
            printf("=");
        }
        printf("\n\n\n");
    }
    drawScreen(MENU_SCREEN);
}

void _drawGameScreen() {
    drawScreen(GAME_OVER_SCREEN);
}

void drawScreen(ScreenType screenType) {
    switch(screenType) {
        case LOGIN_SCREEN:
            _drawLoginScreen();
            break;
        case MENU_SCREEN:
            _drawMenuScreen();
            break;
        case GAME_OVER_SCREEN:
            _drawGameOverScreen();
            break;
        case LEADERBOARD_SCREEN:
            _drawLeaderboardScreen();
            break;
        case GAME_SCREEN:
            _drawGameScreen();
            break;
    }
}

bool authenticateUser(char username[], char password[]) {
    // TODO
    return true;
}