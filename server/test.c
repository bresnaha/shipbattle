#include "server.h"
#include "test_utilities.c"
#include <stdbool.h>

void initialize_player_test (char* username, int socket, char* ip_address) {
	player_t player;
	initialize_player(&player, username, socket, ip_address);
	print_player(player);
}

void bomb_is_valid_move_test (int x, int y) {
	player_t player;
	bomb_t bomb = { .x = x, .y = y };
	initialize_player(&player, "username", 1, "ip_address");
	printf("is_valid_move: %d.\n", is_valid_move(player, bomb, NULL));
}

void ships_is_valid_move_test () {
	ship_t ships[5];
	ships[0] = (ship_t*) {
		.x = 0,
		.y = 0,
		.is_vertical = 1,
		.size = 2
	};

}

/*
 *  main: should contain only calls to individual tests.
 */
int main(int argc, char** argv) {

	printf("\nTEST: initialize_player\n");
	initialize_player_test("username", 1, "ip.add.re.ss");

	printf("\nTEST: is_valid_move\n");
	bomb_is_valid_move_test(0, 0);

	printf("\nTEST: is_valid_move\n");
	bomb_is_valid_move_test(5, 6);

	printf("\nTEST: is_valid_move\n");
	bomb_is_valid_move_test(11, 6);

	printf("\nTEST: is_valid_move\n");
	bomb_is_valid_move_test(5, 11);

	printf("\nTEST: is_valid_move\n");
	bomb_is_valid_move_test(12, 13);




}
