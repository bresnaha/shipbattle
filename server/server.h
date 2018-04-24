#include <pthread.h>

// MACROS
// SHIPS
#define MAX_CONNECTIONS 8

#define SHIP_X_SIZE 1
#define SHIP_Y_SIZE 1
#define SHIP_LENGTH 1
#define SHIP_ORIENTATION_SIZE 1
#define SHIP_INFO_LENGTH 4
#define SHIP_MESSAGE_LENGTH 40
#define BOMB_MESSAGE_LENGTH 10

#define NUMBER_SHIPS 5

#define SHIP_HORIZONTAL 0
#define SHIP_VERTICAL 1

#define BOARD_SIZE 10
// ENUM TYPES ON THE ARRAY REPRESENTATIONS
#define SEA 0
#define SHIP_PRESENT 1
#define BOMB_PRESENT 5

#define PADDING_LENGTH 1
#define USERNAME_LENGTH 10
#define IP_LENGTH 16

typedef struct ship{
  int x;
  int y;
  int is_vertical;
  int size;
} ship_t;

typedef struct bomb{
  int x;
  int y;
} bomb_t;

typedef struct player{
  char username[USERNAME_LENGTH];
  int board[BOARD_SIZE][BOARD_SIZE];
  char ip_address[IP_LENGTH];
  int socket;
  // read status
  pthread_mutex_lock_t lock;
  char* incoming_message;
  bool has_new_message;
  bool live;
} player_t;

int SHIP_SIZES = {2, 3, 3, 4, 5};

//   USER-BASED FUNCTIONS

/*
   initializes the user struct with empty fields but connection, e.g. socket and ip_address
    return: nothing
 */
void initialize_player(player_t player, int socket, char* ip_address);

/*
   populate user's game board; checks if move is valid, and times out on user
    return: true if player has successfully been initialized
 */
bool intialize_board(player_t player, ship_t ships[]);


int extract_int (char* message, int index, int length);

/*
   parses a message into a move (either a bomb or ship placement)
   type has to be sizeof(ships[NUMBER_SHIPS]) or sizeof(bomb_t)!
    return: a boolean, whether move is a bomb == 1 (or ship == 0)
 */
bool parse_message(char* message, bool parse_bomb, void* type);

/*
   manages turn-taking for the specified player
    return: none
*/
char* take_turn(player_t player);

/*
   validates a move
    always take a user, and a bomb OR a ship struct at a time
    return: a boolean, whether a move is valid
 */
bool is_valid_move(player_t player, bomb_t bomb, ship_t ships[]);

/*
   put a ship onto a player's board
    return: nothing
*/
void put_ships(player_t player. ship_t* ships);

void put_bomb(player_t player, bomb_t* bomb);


//   THREAD FUNCTIONS

/*
   initialize and moderate a match between two opponents
    return:
 */
void thread_moderate_match(void* args);

/*
   manages incoming messages from a single player, runs as a thread function
    for the lifespan of the player's connection
    return: none
*/
void thread_player_listener(void* args);

/*
   alternates between writing to both players, runs as a thread function
    for the lifespan of the shorter player's connection
    return: none
*/
void thread_players_writer(void* args);



//    THREAD HELPER FUNCTIONS

/*
   listens for a connection, initializes a user_t by puting connection's
    ip_address and socket
    return: a user_t that holds the connection information
 */
user_t* connection_listner();



//    LISTENER/WRITER FUNCTIONS

/*
   listens for incoming messages from the player
    return: a char*, a message size MESSAGE_LENGTH
 */
char* listen(int length);

/*
   writes an outgoing message to a socket
    returns: a bool, whether the message wrote successfully
    *false reutrn value probably means connection failed
 */
bool write(player_t player, char* message, int length);



//    SERVER-SIDE MISCELLANIES

/*
   checks timestamp and determines if current user has timed-out
    return: boolean, whether user has timed-out
*/
bool time_out();

/*
   picks a random move on the board
    return: a bomb_t, indicates a bomb
*/
bomb_t generate_random_bomb();

/*
   checks whether a goal state has been reached
    return: true if game is on-going/has no winner
 */
int game_not_over();
