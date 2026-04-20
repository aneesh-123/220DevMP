#ifndef MAKE_MOVE_H
#define MAKE_MOVE_H

#include "board.h"
#include "moves.h"

/*
 * Student-implemented move function.
 *
 * Given the current GameState and the player to move (PLAYER_X or PLAYER_O),
 * decide the best move and write the result into the output pointers:
 *
 *   *move_type  -> MOVE_PLACEMENT or MOVE_REMOVAL
 *   *row        -> row index (0..ROWS-1), used only for MOVE_REMOVAL
 *   *column     -> column index (0..COLS-1). For MOVE_PLACEMENT this is the
 *                  column to drop into. For MOVE_REMOVAL this is the column
 *                  of the piece to remove.
 *
 * The returned move must be legal under is_legal_move() in rules.h.
 */
void make_move(GameState *state, int player,
               int *move_type, int *row, int *column);

#endif
