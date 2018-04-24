#include <pthread.h>
#include <stdbool.h>
#include "server.h"

/*
  main listens for incoming connections, initializes a lobby when 
   two users are waiting simultaneously
 */

user_t user1;
user_t user2;
pthread_t match_lobby;     // initialize pthread

int main(int args, char** argv){

  while (user1 == NULL)
    user1 = connection_listner();

  while (user2 == NULL)
    user2 = connection_listner();
  
  pthread_create(match_lobby, pthread_moderate_match, NULL, NULL);

  return 0;
}


pthread_t player_1_listener;
pthread_t player_2_listener;
pthread_t players_writer;

bool player_1_turn;
bool player_1_has_message;

void thread_moderate_match(void* args) {

  pthread_create(player_1_listener, thread_player_listener, player1, NULL);
  pthread_create(player_1_listener, thread_player_listener, player2, NULL);
  pthread_create(players_writer, thread_player_writer, NULL, NULL);

  while (!game_over()) {
    
    while (player_1_turn) {
      
      if (player_1_has_message)
        
    }

  }

}

