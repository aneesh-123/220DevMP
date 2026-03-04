#ifndef AI_MODE4_H
#define AI_MODE4_H

#include "engine.h"
#include "metrics.h"

/*
 * ai_mode4.h — Mode 4: Alpha-Beta + Move Ordering + TT + Iterative Deepening.
 *
 * Searches depth 1, then depth 2, ... up to the requested depth.
 * Each shallower search populates the transposition table, which
 * improves move ordering for deeper searches and can reduce the
 * total nodes expanded.
 *
 * Reuses Mode 3's TT infrastructure (mode3_init / mode3_tt_clear
 * must be called before use).
 */

Move ai_choose_move_mode4(const GameState *state, int depth, Metrics *m);

#endif /* AI_MODE4_H */
