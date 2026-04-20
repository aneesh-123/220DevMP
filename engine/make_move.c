#include "make_move.h"

/*
 * TODO (student): implement your move-selection algorithm here.
 *
 * Inputs:
 *   state   - current game state (read-only; do not mutate)
 *   player  - PLAYER_X or PLAYER_O, the side to move
 *
 * Outputs (write through these pointers):
 *   *move_type - MOVE_PLACEMENT or MOVE_REMOVAL
 *   *row       - row of the piece to remove (ignored for placements)
 *   *column    - column to drop into (placement) or column of the piece
 *                to remove (removal)
 *
 * Your move must be legal. You can verify with is_legal_move() from rules.h.
 */
void make_move(GameState *state, int player,
               int *move_type, int *row, int *column) {
    (void)state;
    (void)player;
    (void)move_type;
    (void)row;
    (void)column;

    /* write your own algo */
}
