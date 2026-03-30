/*
 * Solid standard Connect 4 opponent that ignores the removal twist.
 */

#include "../heuristics.h"

float removal_blind_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.6f * center_control(state, player);
    score += 1.0f * window_strength(state, player);
    score += 1.1f * playable_threats(state, player);
    score += 0.8f * double_threat_pressure(state, player);
    score += 0.7f * line_potential(state, player);

    return score;
}
