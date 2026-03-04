#ifndef AI_MODE3_H
#define AI_MODE3_H

#include "engine.h"
#include "metrics.h"

/*
 * ai_mode3.h — Mode 3: Alpha-Beta + Move Ordering + Transposition Table.
 *
 * Adds Zobrist hashing and a transposition table on top of Mode 2.
 * Previously evaluated positions are cached so that when the same
 * board state is reached via a different move order, the stored
 * result can be reused instead of re-searching the subtree.
 */

/* Call once at startup to initialize Zobrist random keys. */
void mode3_init(void);

/* Clear the transposition table (call before each benchmark run). */
void mode3_tt_clear(void);

/* Choose the best move using alpha-beta + ordering + TT. */
Move ai_choose_move_mode3(const GameState *state, int depth, Metrics *m);

#endif /* AI_MODE3_H */
