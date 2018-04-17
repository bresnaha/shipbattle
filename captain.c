#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int board_height 5
int board_length 5


void display_ships (char** board){
	printf("\n");
	for (int x = 0; x < board_length; x++){
		for(int y = 0; y < board_height; y++){
			printf("%c", board[x][y]);
		}
	}
	printf("\n");

}

int main(int argc, char const *argv[])
{
	char[board_length][board_height] your_ships;


	for (int x = 0; x < board_length; x++)
	{
		for (int y = 0; y < board_height; y++)
		{
			your_ships[x][y] = "~";
		}
	}
	printf("Greetings Captain! What is your name?\n");

	return 0;
}