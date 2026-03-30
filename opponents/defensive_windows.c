/*
 * Defensive opponent that strongly prioritizes blocking live threats.
 */

#include "../heuristics.h"

float defensive_windows_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.2f * center_control(state, player);
    score += 0.8f * window_strength(state, player);
    score += 1.4f * playable_threats(state, player);
    score += 1.3f * immediate_loss_alarm(state, player);

    return score;
}
