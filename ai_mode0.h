#ifndef AI_MODE0_H
#define AI_MODE0_H

#include "engine.h"
#include "metrics.h"

/*
 * ai_mode0.h — Mode 0: Baseline Minimax (no pruning, no caching).
 *
 * Fixed-depth search that exhaustively explores every branch.
 * This is the slowest mode and serves as the baseline for
 * measuring the speedup of later optimizations.
 */

/* Choose the best move for the current player using baseline minimax.
 *   state — current board position (not modified)
 *   depth — how many plies to search
 *   m     — metrics struct (caller must have called metrics_reset)
 *
 * Populates m->nodes_visited and m->depth_completed.
 * Returns the chosen move. */
Move ai_choose_move_mode0(const GameState *state, int depth, Metrics *m);

#endif /* AI_MODE0_H */
