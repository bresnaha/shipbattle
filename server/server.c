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


/*
  main listens for incoming connections, initializes a lobby when
   two users are waiting simultaneously
 */

player_t player1;
player_t player2;
double turn_expire_time;
int listener_socket;
pthread_t match_lobby;
pthread_t player_1_listener;
pthread_t player_2_listener;

int main(int args, char** argv){

  // gather two incoming connections
  open_connection_listener();
  connection_listener(&player1);
  connection_listener(&player2);

  // create thread for lobby
  pthread_create(&match_lobby, NULL, thread_moderate_match, NULL);
  pthread_join(match_lobby, NULL);

  return 0;
}


/*
      Thread functions
*/

void* thread_moderate_match(void* args) {
  // create all required threads
  pthread_create(&player_1_listener, NULL, thread_player_listener, &player1);
  pthread_create(&player_2_listener, NULL, thread_player_listener, &player2);

  // initialize user II: place ships from both players
  bool player_1_initialized = false;
  bool player_2_initialized = false;

  // wait for board setup from both players
  while (!player_1_initialized || !player_2_initialized){
    if (player_1_initialized)
      player_2_initialized = initialize_board(player2);
    else player_1_initialized = initialize_board(player1);
  }

  // begin turns and bomb-dropping
  int exit_state;
  char* message;
  bool player_1_turn = true;

  // check for exit state before next player's turn
  while (!(exit_state = game_not_over(player_1_turn))) {
    write_to_socket(player1, message, SHIP_MESSAGE_LENGTH);
    message = take_turn(player1);
    player_1_turn = !player_1_turn;
    
    // check for exit state before next player's turn
    if (!(exit_state = game_not_over(player_1_turn)))
      break;
    write_to_socket(player2, message, SHIP_MESSAGE_LENGTH);
    message = take_turn(player2);
  }

  // send username of winner
  message = player_1_turn? player2.username : player1.username;
  write_to_socket(player1, message, SHIP_MESSAGE_LENGTH);
  write_to_socket(player2, message, SHIP_MESSAGE_LENGTH);

  free(message);

  // TODO: collect threads
  return 0;
}

void* thread_player_listener(void* args) {
  // TODO: some major work needs to be done here
  player_t* player = (player_t*) args;
  while (player->live) {
    sleep(0);
    // don't read from socket if existing buffer hasn't been read; buffer overrwrite
    if (!player->has_new_message) {
      char* message = calloc(sizeof(char), SHIP_MESSAGE_LENGTH);
      pthread_mutex_lock(&player->lock);
      int bytes_read = read(player->socket, message, sizeof(char)*SHIP_MESSAGE_LENGTH);
      if (bytes_read == -1)
    	player->live = false;
      // bookkeeping
      player->has_new_message = true;
      pthread_mutex_unlock(&player->lock);
    }
  }
  return 0;
}

bool write_to_socket(player_t player, char* message, int length) {
  if (write(player.socket, message, sizeof(char)*length) == -1)
    return false;
  return true;
}

char* read_next(player_t player) {
  pthread_mutex_lock(&player.lock);
  char* message = strndup(player.incoming_message, SHIP_MESSAGE_LENGTH);
  player.has_new_message = false;
  pthread_mutex_unlock(&player.lock);
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
      .sin_port = htons(4444)
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

    struct sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(struct sockaddr_in);

    int socket = accept(listener_socket, (struct sockaddr*)&client_addr, &client_addr_length);
    if(socket == -1)  // error checking
      continue;

    initialize_player(player, strndup(read_next(*player), USERNAME_LENGTH), socket, NULL);
  }
}


/*
      User based functions
*/

void initialize_player(player_t* player, char* username, int socket, char* ip_address) {
  // initialize fields
  player->socket = socket;
  player->incoming_message = NULL;
  player->has_new_message = false;
  player->live = true;
  player->ip_address = strndup(ip_address, IP_LENGTH);
  player->username = strndup(username, USERNAME_LENGTH);
  
  // initialize board
  for (int i = 0; i < BOARD_SIZE; i++)
    for (int j = 0; j < BOARD_SIZE; j++)
      player->board[i][j] = 0;

  // initialize mutex lock
  pthread_mutex_init (&player->lock, NULL);
}

bool initialize_board(player_t player) {
  set_expire_time(WAIT_INIT); // set expiration

  while (!time_out()) {
    sleep(1);

    if (player.has_new_message) {
      char* message = read_next(player);

      if (message != NULL) {
        ship_t ships[NUMBER_SHIPS];
        parse_message(message, NULL, ships);

        if (is_valid_move(player, NULL, ships)) {
          put_ships(player, ships);
          return true;
        }
      }
    }
  }
  return false;
}

bool parse_message(char* message, bomb_t* bomb, ship_t* ships) {
  // TODO: error checking to not overrun message
  // TODO: check for valid user-name
  int offset = USERNAME_LENGTH + PADDING_LENGTH;
  int i = offset;

  if (bomb != NULL) {// dealing with a bomb
    // TODO: parse char to int
    bomb->x = message[i++];
    bomb->y = message[i];
    return true;

  } // parse ship
  for (int j = 0; j < NUMBER_SHIPS; j++){
    ships[j].x = extract_int(message, i++, 1);
    ships[j].y = extract_int(message, i++, 1);
    ships[j].is_vertical = (message[i++] == 'v')? 1 : 0;
    ships[j++].size = extract_int(message, i++, 1);
    i++;
  }
  return true;
}

char* take_turn(player_t player) {
  char* message = (char*) malloc(sizeof(char)*SHIP_MESSAGE_LENGTH);
  set_expire_time(WAIT_TURN);
  // wait for player to take turn
  while (!time_out()) {
    sleep(1);
    if (player.has_new_message) {
      // get new message
      strncpy(message, read_next(player), SHIP_MESSAGE_LENGTH);

      // parse message into a do-able action
      if (message != NULL){
        bomb_t bomb;
        parse_message(message, &bomb, NULL);

        if (is_valid_move(player, &bomb, NULL)) {
          put_bomb(player, &bomb);
          return message;
          
        } else
          write_to_socket(player, "SYSTEM   INV_MOVE", SHIP_MESSAGE_LENGTH);
      }
    }
  }
  // timed_out: take turn for player
  bomb_t *bomb = generate_random_bomb();
  put_bomb(player, bomb);
  // send other player about random bomb
  strncpy(message, player.username, USERNAME_LENGTH);
  message[USERNAME_LENGTH+PADDING_LENGTH] = bomb->x;
  message[USERNAME_LENGTH+PADDING_LENGTH+SHIP_X_SIZE] = bomb->x;
  return message;
}

bool is_valid_move(player_t player, bomb_t* bomb, ship_t ships[]) {
  // bomb check
  if (bomb != NULL)
    return bomb->x < BOARD_SIZE && bomb->y < BOARD_SIZE;

  // ships check
  if (ships != NULL){

    // ship sizes integrity check
    for (int i = 0; i < NUMBER_SHIPS; i++)
      if (SHIP_SIZES[i] != ships[i].size)
        return false;

    // prep
    bool ships_are_valid = true;
    int temp_sea[BOARD_SIZE][BOARD_SIZE];
    for (int i = 0; i < BOARD_SIZE; i++)
      for (int j = 0; j < BOARD_SIZE; j++)
        temp_sea[i][j] = 0;

    // check
    if (ships != NULL) {
      for (int k = 0; k < NUMBER_SHIPS; k++) {
        int x = ships[k].x;
        int y = ships[k].y;
        if (ships[k].is_vertical) { // place ships vertically
          for (int i = 0; i < ships[k].size; i++)
            if (y >= BOARD_SIZE)
              return false;
            else
              temp_sea[x][y++]++;
        } else
          for (int i = 0; i < ships[k].size; i++)
            if (x >= BOARD_SIZE)
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
  }
  return false;
}

void put_ships(player_t player, ship_t* ships) {
  // ships guranteed to be valid and fit
  for (int k = 0; k < NUMBER_SHIPS; k++) {
    int x = ships[k].x;
    int y = ships[k].y;
    if (ships[k].is_vertical)  // place ships vertically
      for (int i = 0; i < ships[k].size; i++)
        player.board[x][y++] = SHIP_PRESENT;
    else
      for (int i = 0; i < ships[k].size; i++)
        player.board[x++][y] = SHIP_PRESENT;
  }
}

void put_bomb(player_t player, bomb_t* bomb) {
  player.board[bomb->x][bomb->y] = BOMB_PRESENT;
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
  return turn_expire_time > get_current_time_ms();
}

void set_expire_time(int seconds){
  turn_expire_time = get_current_time_ms() + seconds*1000;
}

int game_not_over(bool check_player_1) {
  bool player_has_ships = false;

  if (check_player_1) {
    for (int i = 0; i < BOARD_SIZE && !player_has_ships; i++)
      for (int j = 0; j < BOARD_SIZE && !player_has_ships; j++)
        if(player1.board[i][j]) // true if there is a ship at i, j
          player_has_ships = true;
  } else {
    for (int i = 0; i < BOARD_SIZE && !player_has_ships; i++)
      for (int j = 0; j < BOARD_SIZE && !player_has_ships; j++)
        if(player2.board[i][j]) // true if there is a ship at i, j
          player_has_ships = true;
  }
  if (player_has_ships)
    return 1;
  return 0;
}

bomb_t* generate_random_bomb() {
  bomb_t* bomb = (bomb_t*) malloc (sizeof(bomb_t));
  srand(time(0));
  bomb->x = rand() % BOARD_SIZE;
  bomb->y = rand() % BOARD_SIZE;
  return bomb;
}
