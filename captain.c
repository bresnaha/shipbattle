#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define BOARD_LENGTH 10
#define BOARD_HEIGHT 10
#define NUM_SHIPS 4

void init_board(char board[BOARD_LENGTH][BOARD_HEIGHT]) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_LENGTH; x++) {
            board[x][y] = '~'; // water
        }
    }
}

void set_ships(char board[BOARD_LENGTH][BOARD_HEIGHT]) {

    printf("Set up your ships!\n");

    int s = 0;
    while(s < NUM_SHIPS){
        char* ship = NULL;
        size_t linecap = 0;
        printf("Ship: <x-start> <y-start> <length> <orientation>\n");
        getline(&ship, &linecap, stdin);
        printf("Hello, %s\n", ship);

        // To place ship
        // start x, start y, length, orientations (v or h)
        int x = 'D' - 65;
        int y = 5;
        int length = 4;
        char orientation = 'v';

        // check if another boat is already at location
        if (orientation == 'h'){
            for(int i = x; i < x + length; i++){
                board[i][y] = '#';
            } // do something if boat will run off edge
        } else if (orientation == 'v'){
            for(int i = y; i < y + length; i++){
                board[x][i] = '#';
            }
        } else {
            printf("invalid orientation\n");
        }
        s++;
    }
}


void display_ships (char board[BOARD_LENGTH][BOARD_HEIGHT]) {
    printf("  ");
    for(char i = 'A'; i < BOARD_LENGTH + 'A'; i++){
        printf("%c ", i);
    }
    printf(" \n");
    for (int y = 0; y < BOARD_HEIGHT; y++){

        for(int x = 0; x < BOARD_LENGTH; x++){
            if(x == 0)
                printf("%d ", y); // format for over 10
            printf("%c ", board[x][y]);
        }
        printf("\n");
    }
    printf("\n");

}

void update_ships (char xpos, int ypos, char board[BOARD_LENGTH][BOARD_HEIGHT]){
    xpos = (int)xpos - 65;
    if(board[xpos][ypos] == '#'){
        printf("Hit!\n");
        board[xpos][ypos] = '*';
        if(
        // check if ship is sunk
        // check if all ships are sunk
    } else {
        printf("Miss!\n");
        board[xpos][ypos] = '.';
    }
}

int main(int argc, char const *argv[]) {
    char* player1_name = NULL;
    size_t linecap = 0;
    printf("Greetings Captain! What is your name?\n");
    getline(&player1_name, &linecap, stdin);
    printf("Hello, %s\n", player1_name);

    char player1_ships[BOARD_LENGTH][BOARD_HEIGHT];
    init_board(player1_ships);

    set_ships(player1_ships);
    display_ships(player1_ships);

    update_ships ('D' ,5, player1_ships);
    display_ships(player1_ships);

    return 0;
}
