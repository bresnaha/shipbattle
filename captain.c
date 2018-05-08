#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <curses.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "captain.h"
#include "ui.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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


void set_ships(captain_t* captain, char board[BOARD_LENGTH][BOARD_HEIGHT]) {

  ui_add_message("System", "Set up your ships!");
  //Display how to set up ships
  ui_add_message("System", "<x-start>: starting x position ('A' - 'J')");
  ui_add_message("System", "<y-start>: starting y position (0 - 9)");
  ui_add_message("System", "<orientation>: orientation of ship (horiz -> 'h' or vert -> 'v')");

  //display_ships(board);
  int ship_lengths[NUM_SHIPS] = {2, 3, 3, 4, 5}; // int array of lengths of ships
  bool valid; // ship placement is at a valid location

  for(int s = 0; s < NUM_SHIPS; s++){ // for all ships to be set
    valid = false; // initially ship placement is not valid
    while (!valid) { // while ship has not been place in a valid location
      int length = ship_lengths[s];
      // Request location and orientation of a ship
      char* add_ship = "Ship of length  : <x-start> <y-start> <orientation>"; /// fix
      //add_ship[15] = length;
      ui_add_message("System", add_ship);

      char* ship_input = ui_read_input();
      pthread_mutex_lock(&lock);

      // TODO: upper and lower case inputs
      int x = (int)toupper(ship_input[0]) - 65; // at least 0
      int y = atoi(&ship_input[2]);  // at least 0
      char char_orient = toupper(ship_input[4]); // H or V

      int orientation;
      if(char_orient == 'H'){
        orientation = 0; // horizontal
      } else if (char_orient == 'V'){
        orientation = 1; // vertical
      } else {
        orientation = -1; // invalid
      }

      /* add_ship = "Ship: x-start = " + ship_input[0] +
        ", y-start = " + ship_input[2] +
        ", orientation = " + ship_input[4];
      */
      ui_add_message(captain->username, ship_input);

      // check if input is valid
      if(!((x > -1) && (y > -1) && (orientation != -1))) {
        ui_add_message("System", "Invalid input, try again");
      } else {
        if (orientation == 0 && ((x + length -1) < BOARD_LENGTH)) { // horizontal and within bounds
          for(int i = x; i < x + length; i++){
            // if ship already exists here
            if(board[i][y] == '#'){
              // reset back to water
              for(int undo = i - 1; undo >= x; undo--){
                board[undo][y] = '~';
              }
              ui_add_message("System","Ship overlaps with another, try again");
              break;
            } else {
              board[i][y] = '#';
              // if entire ship fits, then ship is valid
              if (i == (x + length -1)){
                captain->send_ships[s][0] = x;
                captain->send_ships[s][1] = y;
                captain->send_ships[s][2] = length;
                captain->send_ships[s][3] = orientation;
                ui_init_ship(length, x, y, orientation);
                valid = true;
              }
            }
          }
        } else if (orientation == 1 && ((y + length-1) < BOARD_HEIGHT)) { // vertical and within bounds
          for(int i = y; i < y + length; i++){
            // ship already exists here
            if(board[x][i] == '#'){
              // reset back to water
              for (int undo = i -1; undo >= y; undo--){
                board[x][undo] = '~';
              }
              ui_add_message("System", "Ship overlaps with another, try again");
              break;
            } else {
              board[x][i] = '#';
              if (i == (y + length - 1)){
                captain->send_ships[s][0] = x;
                captain->send_ships[s][1] = y;
                captain->send_ships[s][2] = length;
                captain->send_ships[s][3] = orientation;
                ui_init_ship(length, x, y, orientation);
                valid = true;
              }
            }
          }
        } else {
          ui_add_message("System", "Ship is out of bounds, try again");
        }
        pthread_mutex_unlock(&lock);
        free(ship_input);
        //display_ships(board);
      }
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

int main(int argc, char** argv) {

    /********************************
    * Parse command line arguments *
    ********************************/
    // (from distributed systems lab)
    if(argc != 4) {
      fprintf(stderr, "Usage: %s <server address> <server port>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    char* server_address = argv[1];
    int server_port = atoi(argv[2]);

    /**********************************
    * Set-up listening client server  *
    ***********************************/
    // Set up a socket (from distributed systems lab)
    int cap_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(cap_socket == -1) {
      perror("Socket");
      exit(2);
    }

    // Listen at this address. We'll bind to port 0 to accept any available port
    struct sockaddr_in addr = {
      .sin_addr.s_addr = INADDR_ANY,
      .sin_family = AF_INET,
      .sin_port = htons(0)
    };

        // Bind to the specified address
    if(bind(cap_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
      perror("bind");
      exit(2);
    }

    // get player's name
    char* your_full = NULL;
    size_t linecap = 0;
    printf("Greetings Captain! What is your name?\n");
    getline(&your_full, &linecap, stdin);

    // only 8 characters for username
    char your_name[USERNAME_LENGTH+1];
    strncpy(your_name, your_full, USERNAME_LENGTH);
    your_name[USERNAME_LENGTH] = 0;

    for(int i = 0; i < USERNAME_LENGTH+1; i++){
      if(your_name[i] == '\n')
        your_name[i] = ' ';
      else if (your_name[i] == '\0') // TODO: fix padding for username
        your_name[i] = ' ';
    }

    // make captain
    captain_t captain1;
    captain1.username = your_name; // cap length 8
    printf("Hello, Captain %s\n", captain1.username);

    char player1_board[BOARD_LENGTH][BOARD_HEIGHT];
    init_board(player1_board);
    ui_init();
    // init ui
    set_ships(&captain1, player1_board);

    // send struct
    write(cap_socket, captain1, sizeof(captain_t));

    //update_ships ('D' ,5, player1_board);
    //display_ships(player1_board);

    // Clean up
    close(cap_socket);

    return 0;
}

/*
    for(int s = 0; s < NUM_SHIPS; s++){
        for(int i = 0; i < 4; i++){
            printf("%d ", captain1.send_ships[s][i]);
        }
        printf("\n");
    }*/
