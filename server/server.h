
//    MACROS

#define MAX_CONNECTIONS 8

//  SHIPS
#define SHIP_X_SIZE 1
#define SHIP_Y_SIZE 1
#define SHIP_LENGTH 1
#define SHIP_ORIENTATION_SIZE 1
#define SHIP_HORIZONTAL 0
#define SHIP_VERTICAL 1
#define SHIP_INFO_LENGTH 4
#define SHIP_MESSAGE_LENGTH 40
#define BOMB_MESSAGE_LENGTH 10
#define NUMBER_SHIPS 5

//  WAITING TIME
#define WAIT_INIT 500
#define WAIT_TURN 30

//  ARRAY REPRESENTATIONS
#define BOARD_SIZE 10
#define SEA 0
#define SHIP_PRESENT 1
#define BOMB_PRESENT 5

//  CONNECTION
#define USERNAME_LENGTH 8
#define PADDING_LENGTH 1
#define IP_LENGTH 16

typedef struct player_msg {
  char username[USERNAME_LENGTH+1];
  //
  //
  // 1 indicates vertical
  //
  int ships[NUMBER_SHIPS][4];
} player_msg_t;

typedef struct bomb_msg {
    char username[USERNAME_LENGTH+1];
    int x;
    int y;
    bool hit;
    int game_over; // 0 - still playing -1 - lose +1 - win
} bomb_msg_t;


typedef struct ship{
  int x;
  int y;
  bool is_vertical;
  int size; // check sum value
} ship_t;


typedef struct player{
  char username[USERNAME_LENGTH+1];
  int board[BOARD_SIZE][BOARD_SIZE];
  char *ip_address;
  int socket;
  // read status
  pthread_mutex_t lock;
  void* incoming_message;
  bool has_new_message;
  bool live;
  bool initialized;
} player_t;

int SHIP_SIZES[] = {2, 3, 3, 4, 5};

//   USER-BASED FUNCTIONS


/*
   populate user's game board; checks if move is valid, and times out on user
    return: true if player has successfully been initialized
 */
bool initialize_board(player_t* player);


int extract_int (char* message, int index, int length);

/*
   parses a message into a move (either a bomb or ship placement)
   type has to be sizeof(ships[NUMBER_SHIPS]) or sizeof(bomb_t)!
    return: a boolean, whether move is a bomb == 1 (or ship == 0)
 */
bool parse_message(player_msg_t* message, ship_t* ships);

/*
   manages turn-taking for the specified player
    return: none
*/
bomb_msg_t* take_turn(player_t* player);

/*
   validates a move
    always take a user, and a bomb OR a ship struct at a time
    return: a boolean, whether a move is valid
 */
bool is_valid_move(bomb_msg_t* bomb, ship_t ships[]);

/*
   put a ship onto a player's board
    return: nothing
*/
void put_ships(player_t* player, ship_t* ships);

void put_bomb(player_t* player, bomb_msg_t* bomb);


//   THREAD FUNCTIONS

/*
   initialize and moderate a match between two opponents
    return:
 */
void* thread_moderate_match(void* args);

/*
   manages incoming messages from a single player, runs as a thread function
    for the lifespan of the player's connection
    return: none
*/
void* thread_player_listener(void* args);


//    THREAD HELPER FUNCTIONS

/*
   Opens a port to listen to incoming connections on
    return: listening socket, an integer
*/
int open_connection_listener();

/*
   listens for a connection, initializes a user_t by puting connection's
    ip_address and socket
    return: none
 */
void connection_listener(player_t *player);



//    LISTENER/WRITER FUNCTIONS

void* read_next(player_t* player, size_t size);


/*
   writes an outgoing message to a socket
    returns: a bool, whether the message wrote successfully
    *false reutrn value probably means connection failed
 */
bool write_to_socket(player_t* player, void* message, size_t size);



//    SERVER-SIDE MISCELLANIES

/*
   get_current_time_ms() and determines if current user has timed-out
    return: true if user has timed-out
*/
bool time_out();

/*
   stores /seconds/ + get_current_time_ms() to turn_expire_time
    returns: nothing
    (checked)
 */
void set_expire_time(int seconds);

/*
   picks a random move on the board
    return: a bomb_t, indicates a bomb
*/
void generate_random_bomb(bomb_msg_t* bomb);

/*
   checks whether a goal state has been reached
    return: true if game is on-going/has no winner
 */
bool game_over(player_t player);

/*
   gets current time in milliseconds
    returns: a double, the current time
    (checked)
 */
double get_current_time_ms();


// TODO: generate_random_bomb() might not be random
