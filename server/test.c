#include "server.h"
#include "test_utilities.c"
#include <stdbool.h>

/*
 *    Checking Tools
 */

void tool_printBoard(int board[BOARD_SIZE][BOARD_SIZE]) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      for (int y = 0; y < BOARD_SIZE; y++)
        printf("%d ", board[x][y]);
      printf("\n");
    }
}

void tool_initBoard(int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int x = 0; x < BOARD_SIZE; x++)
    for (int y = 0; y < BOARD_SIZE; y++)
        board[x][y] = 0;
}

/*
 *    Tests
 */

void test_isValidMove() {
  player_t player;

  tool_initBoard(player.board);
  printf("\n\nTEST: isValidMove()\n");
  printf("Ok Ships\n");
  ship_t ships[NUMBER_SHIPS] = {
    0, 0, 1, 2,
    0, 1, 1, 3,
    0, 2, 1, 3,
    0, 3, 1, 4,
    0, 4, 1, 5
  };
  printf("Expected: True\nActual: %d", is_valid_move(player, NULL, ships));

  tool_initBoard(player.board);
  printf("\n\nTEST: isValidMove()\n");
  printf("Overlapping Ships\n");
  ship_t ships1[NUMBER_SHIPS] = {
    0, 0, 0, 2,
    0, 1, 0, 3,
    0, 2, 0, 3,
    0, 3, 0, 4,
    0, 4, 0, 5
  };
  printf("Expected: False\nActual: %d\n", is_valid_move(player, NULL, ships1));

  tool_initBoard(player.board);
  printf("\n\nTEST: isValidMove()\n");
  printf("Out of Bound Ships\n");
  ship_t ships2[NUMBER_SHIPS] = {
    0, 0, 0, 2,
    9, 9, 0, 3,
    0, 2, 0, 3,
    0, 3, 0, 4,
    0, 4, 0, 5
  };
  printf("Expected: False\nActual: %d\n", is_valid_move(player, NULL, ships2));

}


void test_putShips() {

  player_t player;
  tool_initBoard(player.board);
  printf("\n\nTEST: putShips()\n");
  printf("Expected: Vertical Ships\n");
  tool_printBoard(player.board);
  ship_t ships[NUMBER_SHIPS] = {
    0, 0, 1, 2,
    0, 1, 1, 3,
    0, 2, 1, 3,
    0, 3, 1, 4,
    0, 4, 1, 5
  };
  printf("\n");
  put_ships(&player, ships);
  tool_printBoard(player.board);

  printf("\nExpected: Horizontal Ships");
  tool_initBoard(player.board);
  ship_t ships1[NUMBER_SHIPS] = {
    0, 0, 0, 2,
    1, 1, 0, 3,
    2, 2, 0, 3,
    3, 3, 0, 4,
    4, 4, 0, 5
  };
  printf("\n");
  put_ships(&player, ships1);
  tool_printBoard(player.board);

}

void test_putBomb() {

  player_t player;

  tool_initBoard(player.board);

  printf("\n\nTEST: putBomb()\n");
  tool_printBoard(player.board);
  printf("\n");
  bomb_t bomb;
  generate_random_bomb(&bomb);
  put_bomb(&player, &bomb);
  tool_printBoard(player.board);
  printf("\n");
  generate_random_bomb(&bomb);
  put_bomb(&player, &bomb);
  tool_printBoard(player.board);

}

void test_gameOver() {
  player_t player;

  tool_initBoard(player.board);

  printf("\n\nTEST: game_over()\n");
  printf("Expected: TRUE\nActual: %d", game_over(player));

  player.board[1][2] = 1;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: FALSE\nActual: %d", game_over(player));

  player.board[1][2] = 2;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: TRUE\nActual: %d", game_over(player));

  player.board[1][2] = 0;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: TRUE\nActual: %d", game_over(player));

  player.board[1][2] = 1;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: FALSE\nActual: %d", game_over(player));

  player.board[1][2] = 1;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: FALSE\nActual: %d", game_over(player));

  player.board[9][9] = 1;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: FALSE\nActual: %d", game_over(player));

  player.board[1][2] = 0;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: FALSE\nActual: %d", game_over(player));

  player.board[9][9] = 0;
  printf("\n\nTEST: game_over()\n");
  printf("Expected: TRUE\nActual: %d", game_over(player));

}

double test_getCurrentTimeMs() {
  return get_current_time_ms();
}

void test_setExpireTime(int seconds) {
  set_expire_time(seconds);
}

bool test_setAndTimeOut(int exp_sec, int wait_sec) {
  set_expire_time(exp_sec);
  sleep(wait_sec);
  return time_out();
}

void test(){

  test_gameOver();

  printf("\n\nTEST: generate_random_bomb()\n");
  bomb_t bomb;
  generate_random_bomb(&bomb);
  printf("%d, %d",bomb.x, bomb.y);

  printf("\n\nTEST: get_current_time()\n");
  printf("%f",test_getCurrentTimeMs());

  printf("\n\nTEST: set_expire_time(WAIT_INIT)\n");
  test_setExpireTime(WAIT_INIT);
  printf("%f",turn_expire_time);

  printf("\n\nTEST: set_expire_time(WAIT_TURN)\n");
  test_setExpireTime(WAIT_TURN);
  printf("%f",turn_expire_time);

  printf("\n\nTEST: generate_random_bomb()\n");
  generate_random_bomb(&bomb);
  printf("%d, %d",bomb.x, bomb.y);

  //printf("\n\nTEST: time_out() == true\n");
  //printf("%d",test_setAndTimeOut(1, 2));

  //printf("\n\nTEST: time_out() == false\n");
  //printf("%d",test_setAndTimeOut(2, 1));

  test_putBomb();
  test_putShips();
  test_isValidMove();
}
