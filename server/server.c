#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "server.h"

#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

player_t player1;
player_t player2;
int PORT;
double turn_expire_time;
int listener_socket;
pthread_t match_lobby;
pthread_t player_1_listener;
pthread_t player_2_listener;

/*
  main listens for incoming connections, initializes a lobby when
   two users are waiting simultaneously
 */

void debug(char* message) { printf("  DEBUG: %s\n", message); }

void make_lobby() {

  // gather two incoming connections
  listener_socket = open_connection_listener();
  debug("connection listener opened");
  
  player1.live = false;
  player2.live = false;
    
  connection_listener(&player1);
  connection_listener(&player2);
  debug("established connections with player1 and player2");

  // create thread for lobby
  pthread_create(&match_lobby, NULL, thread_moderate_match, NULL);
  pthread_join(match_lobby, NULL);

}

/*
 *     Main
 */

int main(int argc, char** argv){
  if(argc != 2){
    fprintf(stderr, "Program takes int port");
    return 0;
  }
  PORT = atoi(argv[1]);
  make_lobby();
  return 0;
}

void write_opponents_name(player_t* sender, char* username){
  player_msg_t player;
  strncpy(sender->username, username, USERNAME_LENGTH);
  write_to_socket(sender, (void*)&player, sizeof(player_msg_t));
}

void assign_username_to_struct(char* username, bomb_msg_t* msg) {
  strncpy(msg->username, username, USERNAME_LENGTH);
  msg->username[USERNAME_LENGTH] = '\0';
} 


/*
      Thread functions
*/
  
void* thread_moderate_match(void* args) {

  // create required(listener) threads
  pthread_create(&player_1_listener, NULL, thread_player_listener, &player1);
  pthread_create(&player_2_listener, NULL, thread_player_listener, &player2);
  debug("player_1 & player_2 listeners created");

  // wait for board setup from both players
  while (!player1.initialized || !player2.initialized){
    sleep(1);
    debug("initializing player1 and player2");
    if (player1.initialized)
      player2.initialized = initialize_board(&player2);
    else player1.initialized = initialize_board(&player1);
  }
  debug("player_1 & player_2 initialized");
    
  write_opponents_name(&player1, player2.username);
  write_opponents_name(&player2, player1.username);
  debug("wrote out opponent's name");

  // begin turns and bomb-dropping
  int exit_state;
  bomb_msg_t* message;
  message = take_turn(&player2, &player1);
  debug("player_2 took a turn");
  
  bool player_1_turn = true;

  // check for exit state before next player's turn
  while (!(exit_state = game_over(player1))) { // checks if player1 still has a turn from !losing
    
    write_to_socket(&player1, (void*)message, sizeof(bomb_msg_t)); // writes once to player 1 to report their loss
    write_to_socket(&player2, (void*)message, sizeof(bomb_msg_t)); // writes once to player 2 to report result 
    
    // MEMORY: free(message);
    message = take_turn(&player1, &player2);
    debug("player_1 took a turn");
    
    player_1_turn = !player_1_turn;

    // check for exit state before next player's turn
    if (game_over(player2)) // checks if player2 still has a turn from !losing
      break;
    write_to_socket(&player1, (void*)message, sizeof(bomb_msg_t));
    write_to_socket(&player2, (void*)message, sizeof(bomb_msg_t));
    // MEMORY: free(message);
    message = take_turn(&player2, &player1);
    debug("player_2 took a turn");
  }

  // TODO: prepare winner message to players
  write_to_socket(&player1, (void*)message, sizeof(bomb_msg_t));
  write_to_socket(&player2, (void*)message, sizeof(bomb_msg_t));

  free(message);

  // TODO: collect threads
  return 0;
}
 

void* thread_player_listener(void* args) {
  
  player_t* player = (player_t*) args;
  
  while (player->live) {
    sleep(1);
    // don't read from socket if existing buffer hasn't been read; buffer overrwrite
    if (!player->has_new_message) {
      pthread_mutex_lock(&player->lock);
      
      void* message;
      int bytes_read; 
      
      if (!player->initialized) { // initialize board
        size_t player_msg_size = sizeof(player_msg_t);
        message = malloc(player_msg_size);
        bytes_read = read(player->socket, message, player_msg_size);
        player->incoming_message = message; //(void*) strndup((char*) message, player_msg_size);
        
      } else { // get bomb
        size_t bomb_msg_size = sizeof(bomb_msg_t);
        message = malloc(bomb_msg_size);
        bytes_read = read(player->socket, message, bomb_msg_size);
        player->incoming_message = message; //(void*) strndup((char*) message, bomb_msg_size);
        
      }
      if (bytes_read == -1)
        player->live = false;      
      
      // bookkeeping
      player->has_new_message = true;
      pthread_mutex_unlock(&player->lock);
    }
  }
  return 0;
}
   

bool write_to_socket(player_t* player, void* message, size_t size) {
  if (write(player->socket, message, size) == -1)
    return false;
  return true;
}


void* read_next(player_t* player, size_t size) {
  pthread_mutex_lock(&player->lock);
  // TODO: get rid of this strndup, replace with memcpy or something
  char* message = strndup(player->incoming_message, size);
  player->has_new_message = false;
  pthread_mutex_unlock(&player->lock);
  return message;
}


int open_connection_listener() {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) {
      perror("socket failed");
      exit(2);
    }

    struct sockaddr_in addr = {
      .sin_addr.s_addr = INADDR_ANY,
      .sin_family = AF_INET,
      .sin_port = htons(PORT)
    };

    if(bind(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)))
      return -1;

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(s, (struct sockaddr *)&sin, &len) == -1)
      return -1;

    return s;
}


void connection_listener(player_t* player) {

  while (!player->live) {

    if(listen(listener_socket, 1))  //error checking
      continue;
      
    debug("listening on port");

    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(struct sockaddr_in);
    
    debug("heard connection on port");

    int socket = accept(listener_socket, (struct sockaddr*)&client_addr, &client_addr_length);
    
    debug("accepted connection on port");
    
    if(socket == -1)  // error checking
      continue;
      
    debug("successful connection acceptance on port");

    // INITIALIZE PLAYER STRUCT
    // initialize fields
    player->socket = socket;
    player->has_new_message = false;
    player->initialized = false;
    player->live = true;
    player->incoming_message = NULL;
    player->ip_address = NULL;

    // initialize board
    for (int i = 0; i < BOARD_SIZE; i++)
      for (int j = 0; j < BOARD_SIZE; j++)
        player->board[i][j] = 0;

    // initialize mutex lock
    pthread_mutex_init (&player->lock, NULL);
    
    debug("initialized connection");
  
  }
}

void tool_printBoard(int board[BOARD_SIZE][BOARD_SIZE]) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      for (int y = 0; y < BOARD_SIZE; y++)
        printf("%d ", board[x][y]);
      printf("\n");
    }
}
/*
      User based functions


void tool_print_ships(player_msg_t* msg){
  for (int ship = 0; ship < NUMBER_SHIPS; ship++) 
    for (int elements = 0; elements < 4; elements++)
      printf("%d ", msg[ship][elements]);
}
*/

void temp_initialize_board(player_t* player) {
  ship_t ships[NUMBER_SHIPS] = {
    0, 0, 1, 2,
    0, 1, 1, 3,
    0, 2, 1, 3,
    0, 3, 1, 4,
    0, 4, 1, 5
  };
  put_ships(player, ships);
}

bool initialize_board(player_t* player) {
  set_expire_time(WAIT_INIT);
  
  debug("initializing board");
  
  while (!time_out()) {
    debug("iterating through board init");
    sleep(1);
    if (player->has_new_message) {
      /*
      player_msg_t* message = read_next(player, sizeof(player_msg_t));
      strncpy(player->username, message->username, USERNAME_LENGTH-1);
      player->username[8] = '\0';
      */
      
      debug("read from player_msg_t");

      if (player->incoming_message != NULL) {
        ship_t ships[NUMBER_SHIPS];
        
        pthread_mutex_lock(&player->lock);
        // TODO: get rid of this strndup, replace with memcpy or something
        parse_message(player->incoming_message, ships);
        player->has_new_message = false;
        pthread_mutex_unlock(&player->lock);   

        debug("parsed from player_msg_t");

        //if (is_valid_move(NULL, ships)) {
          put_ships(player, ships);
          tool_printBoard(player->board);
          debug("read valid ships from player_msg_t");
          return true;
        //} else {
        //  temp_initialize_board(player);
        //  return true;
        // }
        
        
        debug("ships invalid");
      } else {
        pthread_mutex_lock(&player->lock);
        player->has_new_message = false;
        pthread_mutex_unlock(&player->lock);   
      }
    }
  }
  return false;
}


bool parse_message(player_msg_t* ships_msg, ship_t* ships) {
  if (ships != NULL) {
    for (int j = 0; j < NUMBER_SHIPS; j++){
      ships[j].x = ships_msg->ships[j][1];
      ships[j].y = ships_msg->ships[j][0];
      ships[j].is_vertical =  ships_msg->ships[j][3];
      ships[j].size = ships_msg->ships[j][2];
    }
    return true;
  }
  return false;
}
 

void copy_message(bomb_msg_t* original, bomb_msg_t* copy) {
  copy->x = original->x;
  copy->y = original->y;
  copy->hit = original->hit;
  copy->game_over = original->game_over;
}

bomb_msg_t* take_turn(player_t* player, player_t* opponent) {
  
  bomb_msg_t* bomb_msg = malloc(sizeof(bomb_msg_t));
  set_expire_time(WAIT_TURN);
  assign_username_to_struct(player->username, bomb_msg);
  bomb_msg->game_over = 0;
  
  // wait for player to take turn
  while (!time_out()) {
    sleep(1);
    
    if (player->has_new_message) {      
      // parse message into a do-able action
      if (player->incoming_message != NULL) {
        
        pthread_mutex_lock(&player->lock);
        //if (is_valid_move(bomb_msg, NULL)) {
        put_bomb(opponent, player->incoming_message);
        
        printf("  DEBUG: message contains bomb at %d %d\n", ((bomb_msg_t*) player->incoming_message)->x, ((bomb_msg_t*) player->incoming_message)->y);
        printf("  DEBUG:  bomb produced a hit %d\n", ((bomb_msg_t*) player->incoming_message)->hit);
        tool_printBoard(opponent->board); // debug
        
        player->has_new_message = false;
        copy_message(player->incoming_message, bomb_msg);
        pthread_mutex_unlock(&player->lock);
        
        return bomb_msg;

        //} else {
          // TODO: write error message to socket
      }  else {
        pthread_mutex_lock(&player->lock);
        player->has_new_message = false;
        pthread_mutex_unlock(&player->lock);
      }
    }
  }
  // timed_out: take turn for player
  generate_random_bomb(bomb_msg);
  put_bomb(opponent, bomb_msg);
  tool_printBoard(opponent->board);
  
  return bomb_msg;
}

bool is_valid_move(bomb_msg_t* bomb, ship_t ships[NUMBER_SHIPS]) {
  // bomb check
  if (bomb != NULL)
    return bomb->x < BOARD_SIZE && bomb->y < BOARD_SIZE;

  // ships check
  if (ships != NULL){

    // ship sizes integrity check
    //for (int i = 0; i < NUMBER_SHIPS; i++)
    //  if (SHIP_SIZES[i] != ships[i].size)
    //    return false;

    // prep
    int temp_sea[BOARD_SIZE][BOARD_SIZE];

    for (int i = 0; i < BOARD_SIZE; i++)
      for (int j = 0; j < BOARD_SIZE; j++)
        temp_sea[i][j] = 0;

      for (int k = 0; k < NUMBER_SHIPS; k++) {
        int x = ships[k].x;
        int y = ships[k].y;
        if (ships[k].is_vertical) { // place ships vertically
          for (int i = 0; i < ships[k].size; i++)
            if (x >= BOARD_SIZE && y >= BOARD_SIZE) // out-of-bound check
              return false;
            else
              temp_sea[x][y++]++;
        } else
          for (int i = 0; i < ships[k].size; i++)
            if (x >= BOARD_SIZE && y >= BOARD_SIZE) // out-of-bound-check
              return false;
            else
              temp_sea[x++][y]++;
      }

      // if any spot has 2 pieces, return false
      for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
          if (temp_sea[i][j] > 1)
            return false;
      return true;
  }

  return false;
}

void put_ships(player_t* player, ship_t ships[NUMBER_SHIPS]) {
  // assumes: ships to be valid and fit into board
  for (int k = 0; k < NUMBER_SHIPS; k++) {
    int x = ships[k].x;
    int y = ships[k].y;
    if (!ships[k].is_vertical)  // place ships vertically
      for (int i = 0; i < ships[k].size; i++)
        player->board[x][y++] = SHIP_PRESENT;
    else
      for (int i = 0; i < ships[k].size; i++)
        player->board[x++][y] = SHIP_PRESENT;
  }
}

void put_bomb(player_t* player, bomb_msg_t* bomb) {
  if (player->board[bomb->x][bomb->y] == SHIP_PRESENT || 
        player->board[bomb->x][bomb->y] == BOMB_PRESENT)
    bomb->hit = true;
  player->board[bomb->x][bomb->y] = BOMB_PRESENT;
}


/*
      Server-side miscellaneous functions
*/

int extract_int(char* message, int index, int length) {
  return atoi(strndup(&message[index], length));
}

double get_current_time_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1000 * tv.tv_sec + tv.tv_usec / 1000; // milliseconds
}


bool time_out() {
  return turn_expire_time < get_current_time_ms();
}


void set_expire_time(int seconds){
  turn_expire_time = get_current_time_ms() + seconds*1000;
}


bool game_over(player_t player) {

    for (int i = 0; i < BOARD_SIZE; i++)
      for (int j = 0; j < BOARD_SIZE; j++)
        if(player.board[i][j] == 1) // true if there is a ship at i, j
          return false;

  return true;
}


void generate_random_bomb(bomb_msg_t* bomb) {
  srand(time(NULL));
  bomb->x = rand() % BOARD_SIZE;
  bomb->y = rand() % BOARD_SIZE;
}


