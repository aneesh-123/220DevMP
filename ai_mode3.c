#include "ai_mode3.h"
#include <stdio.h>

/*
 * Mode 3: Minimax + Alpha-Beta + Move Ordering + Transposition Table.
 * STUB — not yet implemented.
 */

Move ai_choose_move_mode3(const GameState *state, int depth, Metrics *m) {
    (void)state;
    (void)depth;
    (void)m;
    fprintf(stderr, "Error: Mode 3 (alpha-beta + move ordering + TT) is not implemented yet.\n");
    Move dummy = {MOVE_DROP, 0, -1};
    return dummy;
}
