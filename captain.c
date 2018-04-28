#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

#define BOARD_LENGTH 10
#define BOARD_HEIGHT 10
#define NUM_SHIPS 5
//#define SHIP_LENGTHS[NUM_SHIPS] {2,3,3,4,5}

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
                printf("%d ", y);
            printf("%c ", board[x][y]);
        }
        printf("\n");
    }
    printf("\n");

}

void set_ships(char board[BOARD_LENGTH][BOARD_HEIGHT]) {

    printf("Set up your ships!\n");
    //Display how to set up ships
    printf("<x-start>: starting x position ('A' - 'J')\n");
    printf("<y-start>: starting y position (0 - 9)\n");
    printf("<orientation>: orientation of ship (horizontal -> 'h' or vertical -> 'v')\n\n");

    display_ships(board);
    int ship_lengths[NUM_SHIPS] = {2 , 3, 3, 4, 5}; // int array of lengths of ships
    bool valid; // ship placement is at a valid location

    for(int s = 0; s < NUM_SHIPS; s++){ // for all ships to be set
        valid = false; // initially ship placement is not valid
        while (!valid) { // while ship has not been place in a valid location

            // Request location and orientation of a ship
            char* ship = NULL;
            size_t linecap = 0;
            printf("Ship of length %d: <x-start> <y-start> <orientation>\n", ship_lengths[s]);
            getline(&ship, &linecap, stdin);

            // check if input is valid
            int length = ship_lengths[s];
            int x = (int)ship[0] - 65; // at least 0
            int y = ship[2] - 48; // at least 0
            char orientation = ship[4]; // h or v

            printf("Ship: xpos = %d, ypos = %d, orientation = %c\n\n", x, y, orientation);

            if (orientation == 'h' && ((x + length) < 11)) { // horizontal and within bounds
                for(int i = x; i < x + length; i++){
                    // if ship already exists here
                    if(board[i][y] == '#'){
                        // reset back to water
                        for(int undo = i - 1; undo >= x; undo--){
                            board[undo][y] = '~';
                        }
                        printf("Ship overlaps with another, try again\n");
                        break;
                    } else {
                        board[i][y] = '#';
                        // if entire ship fits, then ship is valid
                        if (i == (x + length -1))
                            valid = true;
                    }
                }
            } else if (orientation == 'v' && ((y + length) < 11)) { // vertical and within bounds
                for(int i = y; i < y + length; i++){
                    // ship already exists here
                    if(board[x][i] == '#'){
                        // reset back to water
                        for (int undo = i -1; undo >= y; undo--){
                            board[x][undo] = '~';
                        }
                        printf("Ship overlaps with another, try again\n");
                        break;
                    } else {
                        board[x][i] = '#';
                        if (i == (y + length - 1))
                            valid = true;
                    }
                }
            } else {
                printf("Ship does not fit here, try again\n");
            }
            display_ships(board);
        }
    }
}

void update_ships (char xpos, int ypos, char board[BOARD_LENGTH][BOARD_HEIGHT]){
    xpos = (int)xpos - 65;
    if(board[xpos][ypos] == '#'){
        printf("Hit!\n");
        board[xpos][ypos] = 'X';

        // check if ship is sunk
        // check if all ships are sunk
    } else {
        printf("Miss!\n");
        board[xpos][ypos] = 'O';
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

    update_ships ('D' ,5, player1_ships);
    display_ships(player1_ships);

    return 0;
}
