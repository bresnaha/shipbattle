#include <pthread.h>
#include <stdbool.h>
#include <rand.h>
#include <time.h>
#include <string.h>
#include "server.h"

/*
  main listens for incoming connections, initializes a lobby when
   two users are waiting simultaneously
 */

player_t player1;
player_t player1;
pthread_t match_lobby;     // initialize pthread

int main(int args, char** argv){

  while (player1 == NULL)
    player1 = connection_listner();

  while (player2 == NULL)
    player2 = connection_listner();

  pthread_create(match_lobby, pthread_moderate_match, NULL, NULL);

  return 0;
}

pthread_t player_1_listener;
pthread_t player_2_listener;
pthread_t players_writer;

bool player_1_connected = true;
bool player_2_connected = true;
double turn_expire_time = 0;

/*
      Thread functions
*/

void thread_moderate_match(void* args) {

  // create all required threads
  pthread_create(player_1_listener, thread_player_listener, player1, NULL);
  pthread_create(player_2_listener, thread_player_listener, player2, NULL);
  pthread_create(players_writer, thread_player_writer, NULL, NULL);

  // initialize user II: place ships from both players
  bool player_1_initialized = false;
  bool player_2_initialized = false;

  while (!player_1_initailized || !player_2_initialized){
    if (player_1_initialized)
      player_2_initialized = initialize_board(player2);
    else player_1_initialized = initialize_board(player1);
  }

  // begin turns and bomb-dropping
  int exit_state;
  char* message;

  while (!(exit_state = game_not_over())) {
    write(player1, message, SHIP_MESSAGE_LENGTH);
    message = take_turn(player1);
    if (!(exit_state = game_not_over()))
      break;
    write(player2, message, SHIP_MESSAGE_LENGTH);
    message = take_turn(player2);
  }
  free(message);
  // TODO: send game over message
  write(player1, "someone won", SHIP_MESSAGE_LENGTH);
  write(player2, "someone won", SHIP_MESSAGE_LENGTH);
}

void thread_player_listener(void* args) {
  // TODO: some major work needs to be done here
  player_t* p = (player_t*) args;
  while (p->live) {
    sleep(0);
    // don't read from socket if existing buffer hasn't been read; buffer overrwrite
    if (!has_new_message) {
      char* message = SHIP_MESSAGE_LENGTH;
      pthread_mutex_lock(p->lock);
      int bytes_read = read(p->socket, message, sizeof(char)*SHIP_MESSAGE_LENGTH);
      if (bytes_read == -1)
        p->live = false;
      // bookkeeping
      has_new_message = true;
      pthread_mutex_unlock(p->lock);
    }
  }
}

bool write(player_t player, char* message, int length) {
  if (write(player.socket, message, sizeof(char)*length) == -1)
    return false;
  return true;
}


/*
      User based functions
*/

void initialize_player(player_t player, char* username, int socket, char* ip_address) {
  for (int i = 0; i < BOARD_SIZE; i++)
    for (int j = 0; j < BOARD_SIZE; j++)
      player.board[i][j] = 0;
  player = {
    .username = username,
    .ip_address = ip_address,
    .socket = socket
    .incoming_message = NULL,
    .read = true
  };
  pthread_mutex_init (&player.lock);
}

bool initialize_board(player_t player) {
  turn_expire_time = 0; // set expiry time
  while (!time_out()) { // TODO: borrow time_compare from Charlie
    sleep(1);
    if (has_new_message) {
      pthread_mutex_lock(&player.lock);
      char* message = strndup(player.incoming_message, SHIP_MESSAGE_LENGTH);
      has_new_message = false;
      pthread_mutex_unlock(&player.lock);

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

int extract_int(char* message, int index, int length) {
  return atoi(strndup(message[index], length));
}

bool parse_message(char* message, bool parse_bomb, void* type) {
  // TODO: error checking to not overrun message
  // TODO: check for valid username
  int offset = USERNAME_LENGTH + PADDING_LENGTH;
  int i = offset;

  if (parse_bomb) {// dealing with a bomb
    // TODO: parse char to int
    bomb_t* bomb = (bomb_t*) type;
    bomb->x = message[i++];
    bomb->y = message[i];
    return true;

  } // parse ship
  ship_t* ships = (ship*) type;
  for (int j = 0; j < NUMBER_SHIPS; j++){
    ships[j]->x = extract_int(message[i++], 1);
    ships[j]->y = extract_int(message[i++], 1);
    ships[j]->orientation = extract_int(message[i++], 1);
    ships[j++]->size = extract_int(message[i++], 1);
    i++;
  }
  return true;
}

char* take_turn(player_t player) {
  char* message = (char*) malloc(sizeof(char)*SHIP_MESSAGE_LENGTH);
  // wait for player to take turn
  turn_expire_time = 0; // TODO: set expiry time
  while (!time_out()) {
    sleep(1);
    if (has_new_message) {
      pthread_mutex_lock(&player.lock);
      message = strndup(player.incoming_message, SHIP_MESSAGE_LENGTH);
      has_new_message = false;
      pthread_mutex_unlock(&player.lock);

      if (message != NULL){
        bomb_t bomb;
        parse_message(message, bomb, NULL);
        if (is_valid_move(player, bomb, NULL))
          put_bomb(player, bomb);
          return message;
        else
          write(player.socket, "SYSTEM   INV_MOVE", SHIP_MESSAGE_LENGTH);
      }
    }
  }
  // timed_out: take turn for player
  bomb_t bomb;
  put_bomb(player, generate_random_bomb());
  // TODO: return bomb in string format
  return message;
}

bool is_valid_move(player_t player, bomb_t bomb, ship_t ships[]) {
  // bomb check
  if (bomb != NULL)
    return bomb.x < BOARD_SIZE && bomb.y < BOARD_SIZE;

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
            temp_sea[x][y++]++;
        } else
          for (int i = 0; i < ships[k].size; i++)
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

void put_bomb(player_t player, bomb_t bomb) {
  player.board[bomb.x][bomb.y] = BOMB_PRESENT;
}


/*
      Server-side miscellaneous functions
*/

bool time_out() {
  return true;
}


int game_not_over(bool check_player_1) {
  bool player_has_ships = false;

  if (check_player_1)
    for (int i = 0; i < BOARD_SIZE && !player_1_has_ships; i++)
      for (int j = 0; j < BOARD_SIZE && !player_1_has_ships; j++)
        if(player1.board[i][j]) // true if there is a ship at i, j
          player_has_ships = true;
  else
    for (int i = 0; i < BOARD_SIZE && !player_2_has_ships; i++)
      for (int j = 0; j < BOARD_SIZE && !player_2_has_ships; j++)
        if(player2.board[i][j]) // true if there is a ship at i, j
          player_has_ships = true;

  if (player_has_ships && player1_live)
    return 1;
  return 0;
}

bomb_t* generate_random_bomb() {
  bomb_t* bomb = (bomb_t*) malloc (sizeof(bomb_t));
  srand(time());
  bomb->x = rand() % BOARD_SIZE;
  bomb->y = rand() % BOARD_SIZE;
  return bomb;
}
