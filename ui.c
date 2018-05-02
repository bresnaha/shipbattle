#include "ui.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define WIDTH 78
#define SHIP_HEIGHT 24 // pad of 6 on top and bot
#define CHAT_HEIGHT 5
#define INPUT_HEIGHT 1
#define USERNAME_DISPLAY_MAX 8

#define BOARD_1_Y 6
#define BOARD_2_Y 6
#define BOARD_1_X 12
#define BOARD_2_X 45


WINDOW* mainwin;
WINDOW* shipwin;
WINDOW* chatwin;
WINDOW* inputwin;
  
char* messages[CHAT_HEIGHT];
size_t num_messages = 0;

/**
 * Initialize the chat user interface. Call this once at startup.
 */
void ui_init() {
  // Create the main window
  mainwin = initscr();
  if(mainwin == NULL) {
    fprintf(stderr, "Failed to initialize screen\n");
    exit(EXIT_FAILURE);
  }
  
  // Don't display characters when they're pressed
  noecho();
  
    // Create the ship window
  shipwin = subwin(mainwin, SHIP_HEIGHT + 2, WIDTH + 2, 0, 0);
  box(chatwin, 0, 0);
  // Create the chat window
  chatwin = subwin(mainwin, CHAT_HEIGHT + 2, WIDTH + 2, SHIP_HEIGHT + 2, 0);
  box(chatwin, 0, 0);
  
  // Create the input window
  inputwin = subwin(mainwin, INPUT_HEIGHT + 2, WIDTH + 2, SHIP_HEIGHT + CHAT_HEIGHT + 2, 0);
  box(inputwin, 0, 0);
  
  // print player's empty board
  int rowcount = 0;
  int colcount = 0;
  for (int row = 2+ BOARD_1_Y; row < 12 + BOARD_1_Y; ++row) {
  	for (int col = BOARD_1_X; col < 21 + BOARD_1_X; ++col) {
  		if(col == BOARD_1_X) { 
  			mvaddch(row,col, (char) rowcount+65);
  			colcount = 1;
  		} else if (colcount == 1){
  			mvaddch(row,col, '~');
  			colcount = 0;
  		} else {
  			mvaddch(row,col, ' ');
  			colcount = 1;
  		}
  	}
  	rowcount++;
  }

  // print opponent's empty board
  rowcount = 0;
  colcount = 0;
  for (int row = 2+ BOARD_2_Y; row < 12 + BOARD_2_Y; ++row) {
  	for (int col = BOARD_2_X; col < 21 + BOARD_2_X; ++col) {
  		if(col == BOARD_2_X) { 
  			mvaddch(row,col, (char) rowcount+65);
  			colcount = 1;
  		} else if (colcount == 1){
  			mvaddch(row,col, '~');
  			colcount = 0;
  		} else {
  			mvaddch(row,col, ' ');
  			colcount = 1;
  		}
  	}
  	rowcount++;
  }

  // Refresh the display
  refresh();
}

// Clear the chat window (refresh required)
void ui_clear_chat() {
  for(int i=0; i<WIDTH; i++) {
    for(int j=0; j<CHAT_HEIGHT; j++) {
      mvwaddch(chatwin, 1+j, 1+i, ' ');
    }
  }
}

/**
 * Add a message to the chat window. If username is NULL, the message is
 * indented by two spaces.
 *
 * \param username  The username string. Truncated to 8 characters by default.
 *                  This function does *not* take ownership of this memory.
 * \param message   The message string. This function does *not* take ownership
 *                  of this memory.
 */
void ui_add_message(char* username, char* message) {
  // Clear the chat window
  ui_clear_chat();
  
  // Free the oldest message if it will be lost
  if(num_messages == CHAT_HEIGHT) {
    free(messages[CHAT_HEIGHT-1]);
  } else {
    num_messages++;
  }
  
  // Move messages up
  memmove(&messages[1], &messages[0], sizeof(char*) * (CHAT_HEIGHT - 1));
  
  // Make space for the username and message
  messages[0] = malloc(sizeof(char*) * (WIDTH + 1));
  
  // Keep track of where we are in the message string
  size_t offset = 0;
  
  // Add the username, or indent two spaces if there isn't one
  if(username == NULL) {
    messages[0][0] = ' ';
    messages[0][1] = ' ';
    offset = 2;
  } else if(strlen(username) > USERNAME_DISPLAY_MAX) {
    strncpy(messages[0], username, USERNAME_DISPLAY_MAX-3);
    offset = USERNAME_DISPLAY_MAX-3;
    messages[0][offset++] = '.';
    messages[0][offset++] = '.';
    messages[0][offset++] = '.';
    messages[0][offset++] = ':';
    messages[0][offset++] = ' ';
  } else {
    strcpy(messages[0], username);
    offset = strlen(username);
    messages[0][offset++] = ':';
    messages[0][offset++] = ' ';
  }
  
  // If characters remain, add them in another message
  if(strlen(message) > WIDTH - offset) {
    strncpy(&messages[0][offset], message, WIDTH - offset);
    ui_add_message(NULL, &message[WIDTH - offset]);
  } else {
    strcpy(&messages[0][offset], message);
    
    // Display the messages
    for(int i=0; i<num_messages; i++) {
      mvwaddstr(chatwin, CHAT_HEIGHT - i, 1, messages[i]);
    }
    wrefresh(chatwin);
    wrefresh(inputwin);
  }
}

// Clear the input window (refresh required)
void ui_clear_input() {
  for(int i=0; i<WIDTH; i++) {
    mvwaddch(inputwin, 1, 1+i, ' ');
  }
}

/**
 * Read an input line, with some upper bound determined by the UI.
 *
 * \returns A pointer to allocated memory that holds the line. The caller is
 *          responsible for freeing this memory.
 */
char* ui_read_input() {
  int length = 0;
  int c;
  
  // Allocate space to hold an input line
  char* buffer = malloc(sizeof(char) * (WIDTH + 1));
  buffer[0] = '\0';
  
  // Loop until we get a newline
  while((c = getch()) != '\n') {
    // Is this a backspace or a new character?
    if(c == KEY_BACKSPACE || c == KEY_DC || c == 127) {
      // Delete the last character
      if(length > 0) {
        length--;
        buffer[length] = '\0';
      }
    } else if(length < WIDTH) {
      // Add the new character, unless we're at the length limit
      buffer[length] = c;
      buffer[length+1] = '\0';
      length++;
    }
    
    // Clear the previous input and re-display it
    ui_clear_input();
    mvwaddstr(inputwin, 1, 1, buffer);
    wrefresh(inputwin);
  }
  
  // Clear the input and refresh the display
  ui_clear_input();
  wrefresh(inputwin);
  
  return buffer;
}


/**
 * Adds a ship to the player's display.
 *
 * \param length  	The length if the ship being added.
  * \param col		The com where the top left of the ship is.
 * \param row		The row where the top left of the ship is.
 * \param vert		If the ship is vertical or horizontal.
 *                
 */

void ui_init_ship(int length, int col, int row, bool vert){
	int cur_col = col + BOARD_1_X + 1 + (col/2);
	int cur_row = row + BOARD_1_Y + 2;
	for (int i = 0; i < length; ++i){
		mvaddch(cur_row, cur_col, '#');
		if (vert){
			cur_row++;
		} else {
			cur_col++;
			cur_col++;
		}
	}
	refresh();
}

/**
 * Marks one of your ships as being hit.
 *
 * \param col		The com where the hit is occuring.
 * \param row		The row where the hit is occuring.
 *                
 */

void ui_hit(int col, int row){

	//hit animation
	//do something pretty

	refresh();
}

/**
 * Shows a miss animation.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the miss is occuring.
 *                
 */

void ui_miss(int col, int row){

	//miss animation
	//do something pretty

	refresh();
}

/**
 * Marks one of your opponent's ships as being hit.
 *

 * \param col		The com where the hit is occuring.
 * \param row		The row where the hit is occuring.
 *                
 */
 
void ui_hit_opp(int col, int row){
	mvaddch(row + BOARD_2_Y + 2, col + BOARD_2_X + 1 + (col/2), 'X');
	refresh();
}

/**
 * Marks one of your opponent's squares as being a miss.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the miss is occuring.
 *                
 */

void ui_miss_opp(int col, int row){
	mvaddch(row + BOARD_2_Y + 2, col + BOARD_2_X + 1 + (col/2), 'O');
	refresh();
}





/**
 * Shut down the user interface. Call this once during shutdown.
 */
void ui_shutdown() {
  // Clean up windows and ncurses stuff
  delwin(shipwin);
  delwin(inputwin);
  delwin(chatwin);
  delwin(mainwin);
  endwin();
  refresh();
}