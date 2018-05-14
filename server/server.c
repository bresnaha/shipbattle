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
double turn_expire_time;
int listener_socket;
pthread_t match_lobby;
pthread_t player_1_listener;
pthread_t player_2_listener;

/*
  main listens for incoming connections, initializes a lobby when
   two users are waiting simultaneously
 */

void make_lobby() {

  // gather two incoming connections
  open_connection_listener();
  connection_listener(&player1);
  connection_listener(&player2);

  // create thread for lobby
  pthread_create(&match_lobby, NULL, thread_moderate_match, NULL);
  pthread_join(match_lobby, NULL);

}

/*
 *     Main
 */

int main(int args, char** argv){
  make_lobby();
  return 0;
}


/*
      Thread functions
*/

void* thread_moderate_match(void* args) {

  // create required(listener) threads
  pthread_create(&player_1_listener, NULL, thread_player_listener, &player1);
  pthread_create(&player_2_listener, NULL, thread_player_listener, &player2);

  // initialize user II: place ships from both players
  bool player_1_initialized = false;
  bool player_2_initialized = false;

  // wait for board setup from both players
  while (!player_1_initialized || !player_2_initialized){
    if (player_1_initialized)
      player_2_initialized = initialize_board(&player2);
    else player_1_initialized = initialize_board(&player1);
  }

  // begin turns and bomb-dropping
  int exit_state;
  bomb_msg_t* message;
  bool player_1_turn = true;

  // check for exit state before next player's turn
  while (!(exit_state = game_over(player1))) { // checks if player1 still has a turn from !losing
    write_to_socket(player1, message, sizeof(bomb_msg_t));
    // MEMORY: free(message);
    message = take_turn(player1);
    player_1_turn = !player_1_turn;

    // check for exit state before next player's turn
    if (!(exit_state = game_over(player2))) // checks if player2 still has a turn from !losing
      break;
    write_to_socket(player2, message, sizeof(bomb_msg_t));
    // MEMORY: free(message);
    message = take_turn(player2);
  }

  // TODO: prepare winner message to players
  write_to_socket(player1, message, sizeof(bomb_msg_t));
  write_to_socket(player2, message, sizeof(bomb_msg_t));

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


bool write_to_socket(player_t* player, char* message, size_t size) {
  if (write(player->socket, message, size) == -1)
    return false;
  return true;
}


void* read_next(player_t* player, size_t size) {
  pthread_mutex_lock(&player.lock);
  char* message = strndup(player.incoming_message, size);
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

    // TODO: handle read_next
    initialize_player(player, strndup(read_next(player, sizeof()), USERNAME_LENGTH), socket, NULL);
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


bool initialize_board(player_t* player) {
  set_expire_time(WAIT_INIT);

  while (!time_out()) {
    sleep(1);
    if (player->has_new_message) {
      ships_msg_t* message = read_next(player, sizeof(ships_msg_t));

      if (message != NULL) {
        ship_t ships[NUMBER_SHIPS];
        parse_message(message, NULL, ships);

        if (is_valid_move(NULL, ships)) {
          put_ships(player, ships);
          return true;
        }
      }
    }
  }
  return false;
}


bool parse_message(void* msg, bomb_t* bomb, ship_t* ships) {
  if (bomb != NULL) {// dealing with a bomb
    bomb_msg_t* bomb_msg = (bomb_msg_t*) msg;
    bomb->x = bomb_msg->x;
    bomb->y = bomb_msg->y;
    return true;

  } // parse ship

  if (ships != NULL) {
    for (int j = 0; j < NUMBER_SHIPS; j++){
      ships_msg_t* ships_msg = (ships_msg_t*) msg;
      ships[j].x = ships_msg[j][0];
      ships[j].y = ships_msg[j][1];
      ships[j].is_vertical =  ships_msg[j][2];
      ships[j].size = ships_msg[j][3];
    }
    return true;
  }
  return false;
}


bomb_msg_t* take_turn(player_t* player) {
  bomb_msg_t* msg = malloc(sizeof(bomb_msg_t));
  set_expire_time(WAIT_TURN);
  // wait for player to take turn
  while (!time_out()) {
    sleep(1);
    if (player->has_new_message) {
      // get new message
      strncpy(message, read_next(player, sizeof(bomb_msg_t)), sizeof(bomb_msg_t));

      // parse message into a do-able action
      if (msg != NULL){
        bomb_t bomb;
        parse_message((void*)msg, &bomb, NULL);

        if (is_valid_move(&bomb, NULL)) {
          put_bomb(player, &bomb);
          return msg;

        } else {
          // TODO: write error message to socket
        }
      }
    }
  }
  // timed_out: take turn for player
  bomb_t bomb;
  generate_random_bomb(&bomb);
  put_bomb(&player, &bomb);
  // send other player about random bomb
  strncpy(message, player.username, USERNAME_LENGTH);
  message[USERNAME_LENGTH+PADDING_LENGTH] = bomb.x;
  message[USERNAME_LENGTH+PADDING_LENGTH+SHIP_X_SIZE] = bomb.x;
  return message;
}

bool is_valid_move(bomb_t* bomb, ship_t ships[NUMBER_SHIPS]) {
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

void put_bomb(player_t* player, bomb_t* bomb) {
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


void generate_random_bomb(bomb_t* bomb) {
  srand(time(NULL));
  bomb->x = rand() % BOARD_SIZE;
  bomb->y = rand() % BOARD_SIZE;
}
