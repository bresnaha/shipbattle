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
      char* add_ship = "Ship of length  : <x-start> <y-start> <orientation>"; // TODO: FIX length
      //add_ship[15] = length;
      ui_add_message("System", add_ship);

      char* ship_input = ui_read_input();
      pthread_mutex_lock(&lock);

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
              } // set ship
            } // if horiz
          } // for length
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
                ui_add_message("System", "Ship successfully placed");
                valid = true;
              } // set ship
            } // if vert
          } // for length
        } else {
          ui_add_message("System", "Ship is out of bounds, try again");
        }
        //display_ships(board);
      } // else valid input
      pthread_mutex_unlock(&lock);
      free(ship_input);
    } // while not valid
  } // for every ship
} // set ships

bomb_t prepare_bomb (captain_t* captain){
    bomb_t new_bomb; // new bomb to be prepared

    bool valid = false; // bomb is initially invalid
    while(!valid){
        // Request coordinates for bomb
        ui_add_message("System", "Send Bomb: <x-coordinate> <y-coordinate>");
        char* bomb_input = ui_read_input();

        pthread_mutex_lock(&lock);
        int x = (int)toupper(bomb_input[0]) - 65; // at least 0
        int y = atoi(&bomb_input[2]);  // at least 0

        // Return input to ui
        ui_add_message(captain->username, bomb_input);

        // check if bomb is within bounds
        if(!((x > -1) && (y > -1) && (x < BOARD_LENGTH) && (y < BOARD_HEIGHT))) {
          ui_add_message("System", "Bomb not within bounds, try again");
        } else {
            new_bomb.cap_username = captain->username;
            new_bomb.x = x;
            new_bomb.y = y;
            ui_add_message("System", "Bomb ready for deployment");
            valid = true;
        }
        pthread_mutex_unlock(&lock);
        free(bomb_input);
    }
    return new_bomb;
}

void update_your_board (bomb_t bomb, char board[BOARD_LENGTH][BOARD_HEIGHT]){
    if(board[bomb.x][bomb.y] == '#'){ // hit
        bomb.hit = true;
        ui_hit(bomb.x, bomb.y, board);
    } else { // miss
        bomb.hit = false;
        ui_miss(bomb.x, bomb.y, board);
    }
}

void update_guess_board (bomb_t bomb, char board[BOARD_LENGTH][BOARD_HEIGHT]){
    if(bomb.hit){ // if ship was hit
        board[bomb.x][bomb.y] = 'X';
        ui_hit_opp(bomb.x, bomb.y);
    } else { // miss
        board[bomb.x][bomb.y] = 'O';
        ui_miss_opp(bomb.x, bomb.y);
    }
}

int main(int argc, char** argv) {

    /********************************
    * Parse command line arguments *
    ********************************/
    /* (from distributed systems lab with David)
    if(argc != 4) {
      fprintf(stderr, "Usage: %s <server address> <server port>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    char* server_address = argv[1];
    int server_port = atoi(argv[2]);
    */
    /**********************************
    * Set-up listening client server  *
    ***********************************/
    /* Set up a socket (from distributed systems lab)
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1) {
      perror("Socket");
      exit(2);
    }
    */
    /* Listen at this address. We'll bind to port 0 to accept any available port
    struct sockaddr_in addr = {
      .sin_addr.s_addr = INADDR_ANY,
      .sin_family = AF_INET,
      .sin_port = htons(0)
    };

        // Bind to the specified address
    if(bind(server_socket, (struct sockaddr*)&addr, sizeof(struct sockaddr_in))) {
      perror("bind");
      exit(2);
    }

    // Become a server Socket
    listen(s,2);

    // Get the listening socket info so we can find out which port we're using
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getsockname(s, (struct sockaddr *) &addr, &addr_size);

    int local_port = ntohs(addr.sin_port);
    */
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
    your_name[8] = '\0';

    // make your captain
    captain_t captain1;
    captain1.username = your_name; // cap length 8
    printf("Hello, Captain %s\n", captain1.username);

    // initialize the ui
    ui_init(your_name);

    // make your board and set ships on board
    char your_board[BOARD_LENGTH][BOARD_HEIGHT];
    init_board(your_board);
    set_ships(&captain1, your_board); // set your captain's ships

    // send captain to server
    //write(server_socket, &captain1, sizeof(captain_t));

    // get opponent's name
    captain_t opponent;
    char* opp_name;
    //TODO: thread
    //read(server_socket, &opp_name, sizeof(char)*8);
    ui_set_opp_name(opp_name);
    opponent.username = opp_name;

    // initialize opponent's board
    char guess_board[BOARD_LENGTH][BOARD_HEIGHT];
    init_board(guess_board);

    //while game not done
    char* winner; // username of winner
    bool still_playing = true;
    while (still_playing){
        bomb_t your_bomb = prepare_bomb(&captain1); // prepare captain's bomb
        // send captain's bomb to server
        //write(server_socket, &your_bomb, sizeof(bomb_t));

        // get coordinates from opponent's bomb
        bomb_t opp_bomb;
        //read(server_socket, &opp_bomb, sizeof(bomb_t));

        // update your captain's ships
        update_your_board(opp_bomb, your_board);
        // update your own your guesses board
        update_guess_board(your_bomb, guess_board);

        // check if game is over
        /*
        if(opp_bomb < -99999 || your_bomb < -99999){
            still_playing = false;
        }
        */
    }

    // Display winner's username
    ui_add_message("System:", winner);

    // Clean up
    ui_shutdown();
    //close(server_socket);

    return 0;
}
