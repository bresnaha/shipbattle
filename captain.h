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
    char* username;
    int send_ships[NUM_SHIPS][4];
} captain_t;

typedef struct bomb {
    char* cap_username;
    int x;
    int y;
} bomb_t;



/**
  * Initialize a game board to all water
  *
  * \param board  The 2D char array board.
  */
void init_board(char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * display_ships: display a game board
  *
  * \param board - 2D char array
  */
void display_ships (char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * set_ships: initialize a captain's ships to a game board
  *
  * \param captain  The captain string. Truncated to 8 characters by default
  *                 The captain of the fleet!
  * \param board    The 2D char array board.
  */
void set_ships(captain_t* captain, char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * update_ships: updates a captain's guesses to a game board
  *
  * \param xpos   The int x coordinate of the bomb
  * \param ypos   The int y coordinate of the bomb
  * \param board  The 2D char array board.
  */
void update_ships (int xpos, int ypos, char board[BOARD_LENGTH][BOARD_HEIGHT]);

/**
  * prepare_bomb: prepare a captain's bomb to send to server
  *
  * \param captain  The captain preparing the bomb
  */
bomb_t prepare_bomb (captain_t* captain);
