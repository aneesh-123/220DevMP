#ifndef RULES_H
#define RULES_H

#include "board.h"
#include "moves.h"

/* Maximum possible legal moves: 7 placements + 42 removals. */
#define MAX_LEGAL_MOVES 49

/* Fill moves_out with legal moves. Returns the count. */
int get_legal_moves(const GameState *state, Move *moves_out);

/* Apply a move and return the new state. Does not mutate the input. */
GameState apply_move(const GameState *state, Move move);

/* Check if the board is completely full. */
int is_board_full(const GameState *state);

/* Check if current player has 4 in a row. */
int check_win(const GameState *state);

#endif
