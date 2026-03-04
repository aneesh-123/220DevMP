#ifndef AI_MODE5_H
#define AI_MODE5_H

#include "engine.h"
#include "metrics.h"

/*
 * ai_mode5.h — Mode 5: Gold Standard (fully optimized).
 *
 * Combines every optimization into one search:
 *   - Iterative deepening with aspiration windows
 *   - Transposition table with best-move storage
 *   - Principal Variation Search (PVS)
 *   - Killer move heuristic (2 per ply)
 *   - History heuristic for move ordering
 *
 * Self-contained: has its own TT and Zobrist tables, independent
 * of Mode 3/4, so benchmarks don't interfere with each other.
 */

/* Call once at startup to initialize Zobrist keys. */
void mode5_init(void);

/* Clear the transposition table (call before each benchmark run). */
void mode5_tt_clear(void);

/* Choose the best move using all optimizations. */
Move ai_choose_move_mode5(const GameState *state, int depth, Metrics *m);

#endif /* AI_MODE5_H */
