/*
 * Baseline evaluator: terminal awareness plus stronger positional control.
 */

#include "../heuristics.h"

float baseline_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 2.5f * center_control(state, player);

    return score;
}
