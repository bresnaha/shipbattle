#include "ui.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>

#define WIDTH 78
#define SHIP_HEIGHT 30
#define CHAT_HEIGHT 10
#define INPUT_HEIGHT 1
#define USERNAME_DISPLAY_MAX 8

#define BOARD_1_Y 17
#define BOARD_2_Y 17
#define BOARD_1_X 12
#define BOARD_2_X 45

#define BOARD_LENGTH 10
#define BOARD_HEIGHT 10

#define SLEEP_TIME /*2*/50000000L

pthread_mutex_t ui_lock = PTHREAD_MUTEX_INITIALIZER;

WINDOW* mainwin;
WINDOW* shipwin;
WINDOW* chatwin;
WINDOW* inputwin;
  
char* messages[CHAT_HEIGHT];
size_t num_messages = 0;

/**
 * Initialize the chat user interface. Call this once at startup.
 */
void ui_init(char* username) {
  // ASCII Art by Matthew Bace http://ascii.co.uk/art/battleship
  char* battleship = "                                     |__\n                                     |\\/\n                                     ---\n                                     / | [\n                              !      | |||\n                            _/|     _/|-++'\n                        +  +--|    |--|--|_ |-\n                     { /|__|  |/\\__|  |--- |||__/\n                    +---------------___[}-_===_.'____                 /\\ \n                ____`-' ||___-{]_| _[}-  |     |_[___\\==--            \\/   _\n __..._____--==/___]_|__|_____________________________[___\\==--____,------' .7\n|                                                               charlie-213/\n \\_________________________________________________________________________|";
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
  int letter = 0;
  mvprintw(2,0,battleship);
  for (int row = 2+ BOARD_1_Y; row < 12 + BOARD_1_Y; ++row) {
  	for (int col = BOARD_1_X; col < 1+21 + BOARD_1_X; ++col) {
  		if(col == BOARD_1_X) { 
                  mvaddch(row,col,(char) rowcount+48/*65*/);
  			colcount = 0;
  		} else if (colcount == 1){
        if(row == 2+ BOARD_1_Y && col > BOARD_1_X){
          mvaddch(row-1,col,(char) letter+65);
          letter++;
       }
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
  letter = 0;
  for (int row = 2+ BOARD_2_Y; row < 12 + BOARD_2_Y; ++row) {
  	for (int col = BOARD_2_X; col < 1 + 21 + BOARD_2_X; ++col) {
  		if(col == BOARD_2_X) { 
                  mvaddch(row,col, (char) rowcount+48/*65*/);
  			colcount = 0;
  		} else if (colcount == 1){
        if(row == 2+ BOARD_2_Y && col > BOARD_2_X){
          mvaddch(row-1,col,(char) letter+65);
          letter++;
       }
  			mvaddch(row,col, '~');
  			colcount = 0;
  		} else {
  			mvaddch(row,col, ' ');
  			colcount = 1;
  		}
  	}
  	rowcount++;
  }
 mvprintw(BOARD_1_Y,BOARD_1_X,username);
 mvprintw(BOARD_2_Y,BOARD_2_X,"Opponent");
  // Refresh the display
  refresh();
}

/**
 * Sets opponnent's username.
 *
 * \param username  The username string. Truncated to 8 characters by default.
 *                  This function does *not* take ownership of this memory.
 */
void ui_set_opp_name(char* username){
  mvprintw(BOARD_2_Y,BOARD_2_X,username);
  refresh();
}

// Clear the chat window (refresh required)
void ui_clear_chat() {
  for(int i=0; i<WIDTH; i++) {
    for(int j=0; j<CHAT_HEIGHT-1; j++) {
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
  if(num_messages == CHAT_HEIGHT-1) {
    free(messages[CHAT_HEIGHT-2]);
  } else {
    num_messages++;
  }
  
  // Move messages up
  memmove(&messages[1], &messages[0], sizeof(char*) * (CHAT_HEIGHT - 2));
  
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
      mvwaddstr(chatwin, CHAT_HEIGHT - i-1, 1, messages[i]);
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
	int cur_col = col + BOARD_1_X + 2 + (col+1/2);
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

void ui_hit(int col, int row, char board[BOARD_LENGTH][BOARD_HEIGHT]){
  /*
    /  
  <===≤  ~  # * # ~
    \    ~  # • # ~
         ~  # . # ~
         ~  # @ # ~
         ~  # % # ~
         ~  # & # ~
         ~  # X # ~
  */
	//hit animation
	//do something pretty
  ui_plane(col, row, board, true);
}

void ui_plane(int col, int row, char board [BOARD_LENGTH][BOARD_HEIGHT], bool hit){
  pthread_t bomb_thread;
  int repl_top = 9;
  int repl_mid = 9;
  int repl_bot = 9;
  pos* arg = malloc(sizeof(int)*2);
  arg->row = row;
  arg->col = col;
  int plane_col  = 20;
  int plane_row = row;
  while(plane_col > -14){

  	// add plane
  	// if for printing top wing
  	if((plane_row - 1) >= 0 && (plane_col + 1) >= 0 && (plane_col + 2) <=20){
  		//print the wing('/') at (col-1, row-1)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(-1 + plane_row + 2 + BOARD_1_Y, 2 + plane_col + BOARD_1_X,'/');
      pthread_mutex_unlock(&ui_lock);
  	}
  	// dont let top wing leave trails
  	if((plane_row - 1) >= 0 && (plane_col + 3) >= 1 && (plane_col + 3) <=20){
  		//print the wing('/') at (col-1, row-1)
  		if (plane_col % 2) {
        pthread_mutex_lock(&ui_lock);
  			mvaddch(-1 + plane_row + 2 + BOARD_1_Y, 3 + plane_col + BOARD_1_X, board[repl_top][plane_row - 1]);
        pthread_mutex_unlock(&ui_lock);
        repl_top--;
  		} else {
        pthread_mutex_lock(&ui_lock);
  			mvaddch(-1 + plane_row + 2 + BOARD_1_Y, 3 + plane_col + BOARD_1_X,' ');
        pthread_mutex_unlock(&ui_lock);
  		}
  		
  	}
  	// if for printing plane head 
  	if(plane_col >= 1 && plane_col <=20){
  		//print the head('<') at (col, row)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(plane_row + 2 + BOARD_1_Y, plane_col + BOARD_1_X,'<');
      pthread_mutex_unlock(&ui_lock);
  	}
  	// if for printing plane body 1
  	if((plane_col+1) >= 1 && (plane_col+1) <=20){
  		//print the body('=') at (col-1, row)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(plane_row + 2 + BOARD_1_Y,1 + plane_col + BOARD_1_X,'=');
      pthread_mutex_unlock(&ui_lock);
  	}
  	// if for printing plane body 2
  	if((plane_col+2) >= 1 && (plane_col+2) <=20){
  		//print the body('=') at (col-2, row)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(plane_row + 2 + BOARD_1_Y,2 + plane_col + BOARD_1_X,'=');
      pthread_mutex_unlock(&ui_lock);
  	}
  	// if for printing plane body 3
  	if((plane_col+3) >= 1 && (plane_col+3) <=20){
  		//print the body('=') at (col-3, row)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(plane_row + 2 + BOARD_1_Y,3 + plane_col + BOARD_1_X,'=');
      pthread_mutex_unlock(&ui_lock);
  	}
  	// if for printing plane tail
  	if((plane_col+4) >= 1 && (plane_col+4) <=20){
  		//print the tail('≤') at (col-4, row)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(plane_row + 2 + BOARD_1_Y,4 + plane_col + BOARD_1_X,ACS_LEQUAL);
      pthread_mutex_unlock(&ui_lock);
  	}
  	// dont let plane body leave trails
  	if((plane_col + 5) >= 1 && (plane_col + 5) <=20){
  		//print the wing('/') at (col-1, row-1)
  		if (plane_col % 2) {
        pthread_mutex_lock(&ui_lock);
  			mvaddch(plane_row + 2 + BOARD_1_Y, 5 + plane_col + BOARD_1_X, board[repl_mid][plane_row]);
        pthread_mutex_unlock(&ui_lock);
        repl_mid--;
  		} else {
        pthread_mutex_lock(&ui_lock);
  			mvaddch(plane_row + 2 + BOARD_1_Y, 5 + plane_col + BOARD_1_X,' ');
        pthread_mutex_unlock(&ui_lock);
  		}
  		
  	}

  	// if for printing bot wing
  	if((plane_row + 1) < 10 && (plane_col + 1) >= 0 && (plane_col + 2) <=20){
  		//print the wing('\') at (col+1, row+1)
      pthread_mutex_lock(&ui_lock);
  		mvaddch(1 + plane_row + 2 + BOARD_1_Y,2 + plane_col + BOARD_1_X,'\\');
      pthread_mutex_unlock(&ui_lock);
  	}
  	// dont bot wing leave trails
  	if((plane_row + 1) < 10 && (plane_col + 3) >= 1 && (plane_col + 3) <=20){
  		//print the wing('/') at (col+1, row-1)
  		if (plane_col % 2) {
        pthread_mutex_lock(&ui_lock);
  			mvaddch(1 + plane_row + 2 + BOARD_1_Y, 3 + plane_col + BOARD_1_X, board[repl_bot][plane_row + 1]);
        pthread_mutex_unlock(&ui_lock);
        repl_bot--;
  		} else {
        pthread_mutex_lock(&ui_lock);
  			mvaddch(1 + plane_row + 2 + BOARD_1_Y, 3 + plane_col + BOARD_1_X,' ');
        pthread_mutex_unlock(&ui_lock);
  		}
  		
  	}
  	// if for starting dropping bomb
  	if(plane_col == ((col * 2) -5 )){
  		// if its a hit or a miss
  		if (hit) {
  			if(pthread_create(&bomb_thread, NULL, ui_hit_bomb, (void*) arg)) {
    		perror("pthread_create failed");
    		exit(2);
 		 }
  		} else {
  			if(pthread_create(&bomb_thread, NULL, ui_miss_bomb, (void*) arg)) {
    		perror("pthread_create failed");
    		exit(2);
  		}
  	}
  }
  	refresh();
  	nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  	plane_col--;
  }
  if(pthread_join(bomb_thread, NULL)) {
    perror("pthread_join failed");
    exit(2);
  }
 }


void* ui_hit_bomb(void* arg){
  pos* hit_location = (pos*)arg;
  int col = hit_location->col + 1;
  int row = hit_location->row;
  free(arg);
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '*');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '.');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2+ BOARD_1_X, '@');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '%');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '&');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, 'X');
  pthread_mutex_unlock(&ui_lock);
  refresh();
  return NULL;
}

void* ui_miss_bomb(void* arg){
  pos* hit_location = (pos*)arg;
  int col = hit_location->col;
  int row = hit_location->row;
  free(arg);
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '*');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '.');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '@');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, 'W');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, 'w');
  pthread_mutex_unlock(&ui_lock);
  nanosleep((const struct timespec[]){{0, SLEEP_TIME}}, NULL);// sleep for half a second
  pthread_mutex_lock(&ui_lock);
  mvaddch(row + BOARD_1_Y + 2, col * 2 + BOARD_1_X, '~');
  pthread_mutex_unlock(&ui_lock);
  refresh();
  return NULL;
}

/**
 * Shows a miss animation.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the miss is occuring.
 *                
 */

void ui_miss(int col, int row, char board[BOARD_LENGTH][BOARD_HEIGHT]){
 /*
    /  
  <===≤  ~  ~ * ~ ~
    \    ~  ~ • ~ ~
         ~  ~ . ~ ~
         ~  ~ W ~ ~
         ~  ~ w ~ ~
         ~  ~ ~ ~ ~
  */
  //miss animation
  //do something pretty
  ui_plane(col+1, row, board, false);
}

/**
 * Marks one of your opponent's ships as being hit.
 *

 * \param col		The com where the hit is occuring.
 * \param row		The row where the hit is occuring.
 *                
 */
 
void ui_hit_opp(int col, int row){
  mvaddch(row + BOARD_2_Y + 2, col * 2 + BOARD_2_X, 'X');
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
	mvaddch(row + BOARD_2_Y + 2, col * 2 + BOARD_2_X, '0');
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
