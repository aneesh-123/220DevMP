/*
 * V1: Terminal states + center control only.
 *
 * Knows wins/losses and prefers central columns, but has no awareness
 * of threats, partial connections, or removal timing.
 * Baseline to show that positional knowledge alone is not enough.
 */

#include "../heuristics.h"

float v1_center_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.5f * center_control(state, player);

    return score;
}
