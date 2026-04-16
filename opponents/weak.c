/*
 * Weak opponent: terminal bonus + slight center column preference.
 * Should be beatable by any reasonable multi-heuristic evaluator.
 */

#include "../heuristics.h"

float weak_evaluate(const GameState *state, int player) {
    return terminal_state_bonus(state, player);
}
