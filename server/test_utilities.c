#include <stdio.h>
#include <stdbool.h>
#include "server.h"

void print_board(int board[][], int board_size) {
	for (int i = 0; i < board_size; i++) {
		for (int j = 0; j < board_size; j++)
			printf("%d ", board[i][j]);
		printf("\n");
	}
}


void print_board_inside_player(player_t* player, int board_size){
	print_board(player->board, board_size);
}


void print_player(player_t* player) {
	// formatting string depends should correspond to server.h declaration

	// username
	if (player->username == NULL)
		fprintf(stderr, "Username is null.");
	else printf("  player username: %.10s\n", player->username);

	// board
	print_board(player->board, BOARD_SIZE);

	// ip_address
	if (player->ip_address == NULL)
		fprintf(stderr, "ip_address is null.");
	else printf("  ip_address: %.5s\n", player->ip_address);

	// socket number
	printf("  socket number: %d\n", player->socket);

	// incoming message
	if (player->incoming_message == NULL)
		fprintf(stderr, "incoming_message is null.");
	else printf("  Incoming Message:\n   %.40s", player->incoming_message);

	// has_new_message
	printf("  Socket has new message: %d\n", player->has_new_message);

	// socket is live
	printf("  Socket is live: %d\n", player->live);
}
