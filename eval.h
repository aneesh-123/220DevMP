#ifndef EVAL_H
#define EVAL_H

#include "engine.h"

/*
 * eval.h — Heuristic board evaluation for Modified Connect 4.
 *
 * All modes share the same evaluation function so that benchmark
 * comparisons are fair (the only variable is the search strategy).
 *
 * Scoring is from X's perspective:
 *   positive = good for X, negative = good for O.
 */

/* Large score constants for terminal states. */
#define SCORE_WIN   100000
#define SCORE_INF  1000000

/* Evaluate the board heuristically (non-terminal positions).
 * Uses a sliding-window approach over all possible 4-cell lines:
 *   +100  for 3-in-a-row with 1 empty
 *    +10  for 2-in-a-row with 2 empty
 *   -120  for opponent 3-in-a-row with 1 empty (slightly higher weight)
 *    -12  for opponent 2-in-a-row with 2 empty
 *   +6/-6 bonus for center column occupancy
 */
int evaluate(const GameState *s);

#endif /* EVAL_H */
