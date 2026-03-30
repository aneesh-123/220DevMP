/*
 * Greedy opponent that builds its own windows but under-defends.
 */

#include "../heuristics.h"

float greedy_windows_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.4f * center_control(state, player);
    score += 1.3f * window_strength(state, player);
    score += 0.5f * playable_threats(state, player);

    return score;
}
