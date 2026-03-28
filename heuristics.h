#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "engine/board.h"

/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * Each function takes a game state and a player, and returns a float score.
 * Combine these in your evaluators to build a strong bot.
 *
 * ADD YOUR OWN HEURISTIC FUNCTIONS HERE.
 */

/* Returns a large bonus/penalty for terminal (win/loss) states, else 0. */
float terminal_state_bonus(const GameState *state, int player);

#endif
