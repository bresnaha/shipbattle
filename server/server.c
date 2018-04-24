#include <pthread.h>
#include <stdbool.h>
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
  while (!game_over()) {
    take_turn(player1);
    if (!game_over())
      break;
    take_turn(player2);
  }
  // send game over message
}


void take_turn(player_t player) {
  while (!time_out()) {
    char* message;
    if (message != NULL){
      bomb_t bomb;
      parse_message(message, bomb, NULL);
      if (is_valid_move(player, bomb, NULL))
        put_bomb(player, bomb);
        return true;
    }
  }
  // timed_out: take turn for player
  bomb_t bomb;
  put_bomb(player, generate_random_bomb());
}

bool initialize_board(player_t player) {
  while (!time_out()) { // TODO: borrow time_compare from Charlie
    // TODO: get message from listener
    char* message; // read next thing in buffer
    if (message != NULL) {
      ship_t ships[NUMBER_SHIPS];
      parse_message(message, NULL, ships);
      if (is_valid_move(player, NULL, ships)) {
        put_ships(player, ships);
        return true;
      }
    }
  }
  return false;
}


int game_over() {
  // TODO: check conditions
  bool player_1_has_ships = false;
  bool player_2_has_ships = false;

  for (int i = 0; i < BOARD_SIZE && !player_1_has_ships; i++)
    for (int j = 0; j < BOARD_SIZE && !player_1_has_ships; j++)
      if(user1.board[i][j]) // true if there is a ship at i, j
        player_1_has_ships = true;

  for (int i = 0; i < BOARD_SIZE && !player_2_has_ships; i++)
    for (int j = 0; j < BOARD_SIZE && !player_2_has_ships; j++)
      if(user2.board[i][j]) // true if there is a ship at i, j
        player_2_has_ships = true;

  if (player_1_has_ships && !player_2_has_ships)
    return 11;

  if (!player_1_has_ships && player_2_has_ships)
    return 22;

  if (player_1_has_ships && player_2_has_ships && player_1_alive && player_2_alive)
    return 1;
}


bomb_t* generate_random_bomb() {
  bomb_t* bomb = (bomb_t*) malloc (sizeof(bomb_t));
  srand(time());
  bomb->x = rand() * BOARD_SIZE;
  bomb->y = rand() * BOARD_SIZE;
  return bomb;
}
