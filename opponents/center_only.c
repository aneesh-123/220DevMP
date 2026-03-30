/*
 * Center-focused opponent with very limited tactical understanding.
 */

#include "../heuristics.h"

float center_only_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 2.8f * center_control(state, player);

    return score;
}
