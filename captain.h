#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

#define BOARD_LENGTH 10
#define BOARD_HEIGHT 10
#define NUM_SHIPS 5
#define USERNAME_LENGTH 8

typedef struct captain {
    char username[USERNAME_LENGTH+1];
    int send_ships[NUM_SHIPS][4];
} captain_t;

typedef struct bomb {
    char cap_username[USERNAME_LENGTH+1];
    int x;
    int y;
    bool hit;
    int game_over; // 0 - still playing -1 - lose +1 - win
} bomb_t;

/**
  * init_board:   Initialize a game board to all water '~'
  *
  * \param board  The 2D char array game board
  */
void init_board(char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * display_ships:  Display a game board
  *
  * \param board    The 2D char array game board
  */
void display_ships (char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * set_ships:      Initialize a captain's ships to a game board
  *
  * \param captain  The captain of your fleet!
  * \param board    The 2D char array game board
  */
void set_ships(captain_t* captain, char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * prepare_bomb:   Prepare a captain's bomb to send to server
  *
  * \param captain  The captain preparing the bomb
  */
bomb_t prepare_bomb (captain_t* captain);

/**
  * update_your_board: Updates a captain's guesses to a game board
  *
  * \param bomb   The bomb the opponent sent
  * \param board  The 2D char array game board
  */
void update_ship_board (bomb_t opp_bomb, char your_board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * update_guess_board Updates a captain's guesses to a game board
  *
  * \param bomb   The bomb the captain sent
  * \param board  The 2D char array game board
  */

void update_guess_board (bomb_t your_bomb, char guess_board[BOARD_LENGTH][BOARD_HEIGHT]);
