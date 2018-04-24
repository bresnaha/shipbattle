#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

#define BOARD_LENGTH 10
#define BOARD_HEIGHT 10
#define NUM_SHIPS 5

void init_board(char board[BOARD_LENGTH][BOARD_HEIGHT]) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_LENGTH; x++) {
            board[x][y] = '~'; // water
        }
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

void set_ships(char board[BOARD_LENGTH][BOARD_HEIGHT]) {

    printf("Set up your ships!\n");
    display_ships(board);
    int ship_lengths[NUM_SHIPS] = {2 , 3, 3, 4, 5};

    // example

    for(int s = 0; s < NUM_SHIPS; s++){
        bool valid = false; // initialize unknown to false
        while(!valid){
            char* ship = NULL;
            size_t linecap = 0;
            printf("Ship of length %d: <x-start> <y-start> <orientation>\n", ship_lengths[s]);
            getline(&ship, &linecap, stdin);

            int length = ship_lengths[s];
            int x = (int)ship[0] - 65;
            int y = ship[2] - 48;
            char orientation = ship[4];
            printf("Ship: xpos = %d, ypos = %d, orientation %c\n", x, y, orientation);

            if (orientation == 'h' && (x + length) < 9){ // horizontal and within bounds
                for(int i = x; i < x + length; i++){
                    if(board[i][y] == '#'){ // ship already exists here
                        printf("retry\n");
                    } else {
                        board[i][y] = '#';
                        valid = true;
                    }
                }
            } else if (orientation == 'v' && (y + length) < 9){ // vertical and within bounds
                for(int i = y; i < y + length; i++){
                    if(board[x][i] == '#'){ // ship already exists here
                        printf("retry\n");
                    } else {
                        board[x][i] = '#';
                        valid = true;
                    }
                }
            } else {
                printf("invalid position, try again\n");
            }
            display_ships(board);
        }
    }
}

void update_ships (char xpos, int ypos, char board[BOARD_LENGTH][BOARD_HEIGHT]){
    xpos = (int)xpos - 65;
    if(board[xpos][ypos] == '#'){
        printf("Hit!\n");
        board[xpos][ypos] = '*';

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
