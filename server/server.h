
// MACROS
// SHIPS
#define SHIP_PADDING_SIZE 1
#define SHIP_X_SIZE 1
#define SHIP_Y_SIZE 1
#define SHIP_LENGTH 1
#define SHIP_ORIENTATION_SIZE 1
#define SHIP_MESSAGE_LENGTH 40
#define BOMB_MESSAGE_LENGTH 10

#define SHIP_HORIZONTAL 0
#define SHIP_VERTICAL 1

#define BOARD_SIZE 10
// ENUM TYPES ON THE ARRAY REPRESENTATIONS
#define SEA 0
#define SHIP_PRESENT 1
#define BOMBED 5

#define USERNAME_LENGTH 10
#define IP_LENGTH 16

typedef struct ship{
  int x;
  int y;
  int orientation;
  int size;
} ship_t;

typedef struct bomb{
  int x;
  int y;
} bomb_t;

typedef struct user{
  char username[USERNAME_LENGTH];
  int board[BOARD_SIZE][BOARD_SIZE];
  int ships_remaining;                // 0 if none, which is false
  char ip_address[IP_LENGTH];
  int socket;
} user_t;


//   USER-BASED FUNCTIONS

/* 
   initializes the user struct
    return: nothing
 */
void initialize_user(user_t *user);

/* 
   put a ship onto a player's board
    return: nothing
*/
void put_ship(user_t user, ship_t ship);

/* 
   validates a move
    type 
    return: a boolean, whether a move is valid
 */
bool is_valid_move(int type, int x, int y, char* extra);



//   THREAD FUNCTIONS

/* 
   initialize a match
    return: 
 */
void thread_initialize_match(void* args);

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
bool write(int length);



//    SERVER-SIDE MISCELLANIES

/*
   checks timestamp and determines if current user has timed-out
    return: boolean, whether user has timed-out
*/
bool timed_out();

/*
   picks a random move on the board
    return: a bomb_t, indicates a bomb
*/
bomb_t generate_random_bomb();






  
