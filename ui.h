#ifndef UI_H
#define UI_H
#include <stdbool.h>
/**
 * Initialize the chat user interface. Call this once at startup.
 * \param username  The username string. Truncated to 8 characters by default.
 *                  This function does *not* take ownership of this memory.
 */
void ui_init(char* username);

/**
 * Add a message to the chat window. If username is NULL, the message is
 * indented by two spaces.
 *
 * \param username  The username string. Truncated to 8 characters by default.
 *                  This function does *not* take ownership of this memory.
 * \param message   The message string. This function does *not* take ownership
 *                  of this memory.
 */

/**
 * Sets opponnent's username.
 *
 * \param username  The username string. Truncated to 8 characters by default.
 *                  This function does *not* take ownership of this memory.
 */
void ui_set_opp_name(char* username);

void ui_add_message(char* username, char* message);

/**
 * Read an input line, with some upper bound determined by the UI.
 *
 * \returns A pointer to allocated memory that holds the line. The caller is
 *          responsible for freeing this memory.
 */
char* ui_read_input();

/**
 * Adds a ship to the player's display.
 *
 * \param length  	The length if the ship being added.
 * \param col		The com where the miss is occuring.
 * \param row		The row where the top left of the ship is.
 * \param vert		If the ship is vertical or horizontal.
 *                
 */
void ui_init_ship(int length, int col, int row, bool vert);

/*helper function for ui_hit and ui_miss*/
void ui_plane(int col, int row, char ** board, bool hit);
void* ui_hit_bomb(void* arg);
void* ui_miss_bomb(void* arg);

/**
 * Marks one of your ships as being hit.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the hit is occuring.
 *                
 */

void ui_hit(int col, int row);

/**
 * Displays a missed attack animation.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the miss is occuring.
 *                
 */

void ui_miss(int col, int row);

/**
 * Marks one of your opponent's ships as being hit.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the hit is occuring.
 *                
 */
 
void ui_hit_opp(int col, int row);

/**
 * Marks one of your opponent's squares as being a miss.
 *
 * \param col		The com where the miss is occuring.
 * \param row		The row where the miss is occuring.
 *                
 */

void ui_miss_opp(int col, int row);




/**
 * Shut down the user interface. Call this once during shutdown.
 */
void ui_shutdown();

#endif