#include "ai_mode4.h"
#include <stdio.h>

/*
 * Mode 4 (optional): Mode 3 + Iterative Deepening.
 * STUB — not yet implemented.
 */

Move ai_choose_move_mode4(const GameState *state, int depth, Metrics *m) {
    (void)state;
    (void)depth;
    (void)m;
    fprintf(stderr, "Error: Mode 4 (iterative deepening) is not implemented yet.\n");
    Move dummy = {MOVE_DROP, 0, -1};
    return dummy;
}
