#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>

char * serverAddress;
uint16_t serverPort;

int sockfd;
struct sockaddr_in socketAddress;
struct hostent * server;

char username[16];
int session = -1;

ClientGameState * gameState;

int main(int argc, char ** argv) {
    switch (argc) {
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

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Failed to create socket");
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Unknown host\n");
        exit(0);
    }

    bzero((char *) &socketAddress, sizeof(socketAddress));
    socketAddress.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &socketAddress.sin_addr.s_addr, (size_t) server->h_length);
    socketAddress.sin_port = htons(serverPort);

    // Connect to the server, and fail with the appropriate error.
    if (connect(sockfd, (const struct sockaddr *) &socketAddress, sizeof(socketAddress)) < 0) {
        error("Failed to connect to server");
    }

    // Draw the initial welcome text
    drawWelcomeText();

    drawScreen(LOGIN_SCREEN);

    DataPacket packet;
    packet.type = CLOSE_CLIENT_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);
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

        status = scanf("%d", &choice);
        if (status == 0) {
            // If the input was invalid, clear out the input buffer.
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
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

void _drawWinScreen() {
    printf("\nGame over\n\n\n");
    printf("Well done %s! You won this round of Hangman!\n\n\n", username);
    drawScreen(MENU_SCREEN);
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
    startGame();
    while (gameState->remainingGuesses > 0 && !gameState->won) {
        printf("\n");
        for (int i = 0; i < 20; i++) {
            printf("-");
        }
        printf("\n\n\n");
        printf("Guessed letters: %s\n\n", gameState->guessedLetters);
        printf("Number of guesses left: %d\n\n", gameState->remainingGuesses);
        printf("Word: %s\n\n", gameState->currentGuess);
        printf("Enter your guess -> ");
        char choice;
        int status = scanf("%c\n", &choice);
        if (status == 0) {
            // If the input was invalid, clear out the input buffer.
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        } else {
            guessCharacter(choice);
        }
    }

    drawScreen(gameState->won ? WIN_SCREEN : GAME_OVER_SCREEN);
}

void drawScreen(ScreenType screenType) {
    switch (screenType) {
        case LOGIN_SCREEN:
            _drawLoginScreen();
            break;
        case MENU_SCREEN:
            _drawMenuScreen();
            break;
        case GAME_OVER_SCREEN:
            _drawGameOverScreen();
            break;
        case WIN_SCREEN:
            _drawWinScreen();
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
    LoginDetailsPayload payload;
    strcpy(payload.username, username);
    strcpy(payload.password, password);

    DataPacket dataPacket;
    dataPacket.type = LOGIN_PACKET;

    send(sockfd, &dataPacket, sizeof(dataPacket), 0);
    send(sockfd, &payload, sizeof(LoginDetailsPayload), 0);

    DataPacket * inputPacket = malloc(sizeof(DataPacket));
    recv(sockfd, inputPacket, sizeof(DataPacket), 0);

    if (inputPacket->type != LOGIN_RESPONSE_PACKET) {
        printf("Got packet %d.", inputPacket->type);
        error("Received wrong packet. Login response packet expected");
    }
    session = inputPacket->session;
    free(inputPacket);

    LoginResponsePayload * responsePayload = malloc(sizeof(LoginResponsePayload));
    recv(sockfd, responsePayload, sizeof(LoginResponsePayload), 0);
    bool success = responsePayload->success;
    free(responsePayload);

    return success;
}

void _receiveGameState() {
    DataPacket * inputPacket = malloc(sizeof(DataPacket));
    recv(sockfd, inputPacket, sizeof(DataPacket), 0);

    if (inputPacket->type != STATE_RESPONSE_PACKET) {
        if (inputPacket->type == INVALID_GUESS_PACKET) {
            printf("\n\nInvalid guess. Guesses must be lowercase letters!\n\n");
            free(inputPacket);
            return;
        } else {
            printf("Got packet %d.", inputPacket->type);
            error("Received wrong packet. State response packet expected");
        }
    }
    free(inputPacket);

    ClientGameState * responsePayload = malloc(sizeof(ClientGameState));
    recv(sockfd, responsePayload, sizeof(ClientGameState), 0);
    gameState = malloc(sizeof(ClientGameState));
    memcpy(gameState, responsePayload, sizeof(ClientGameState));
    free(responsePayload);
}

void startGame() {
    DataPacket packet;
    packet.type = START_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);

    _receiveGameState();
}

void guessCharacter(char character) {
    TakeTurnPayload takeTurnPayload;
    takeTurnPayload.guess = character;

    DataPacket packet;
    packet.type = GUESS_PACKET;
    packet.session = session;

    send(sockfd, &packet, sizeof(packet), 0);
    send(sockfd, &takeTurnPayload, sizeof(TakeTurnPayload), 0);

    _receiveGameState();
}