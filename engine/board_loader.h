#ifndef BOARD_LOADER_H
#define BOARD_LOADER_H

#include "board.h"

#define MAX_BOARD_STATES 64
#define MAX_STATE_NAME 64

/* Load all .json board states from the given directory.
 * Fills states_out and names_out arrays.
 * Returns the number of states loaded. */
int load_board_states(const char *dir,
                      GameState *states_out,
                      char names_out[][MAX_STATE_NAME],
                      int max_states);

#endif
