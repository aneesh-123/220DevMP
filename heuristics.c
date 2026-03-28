/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * ADD YOUR OWN HEURISTIC FUNCTIONS HERE.
 * Each function should:
 *   - Take (const GameState *state, int player)
 *   - Return a float score
 *   - Be declared in heuristics.h
 */

#include "heuristics.h"

float terminal_state_bonus(const GameState *state, int player) {
    if (!state->is_terminal)
        return 0.0f;

    if (state->winner == player)
        return 10000.0f;
    else if (state->winner != WINNER_NONE)
        return -10000.0f;
    else
        return 0.0f; /* draw */
}
