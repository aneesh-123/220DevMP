/*
 * Tactical opponent that hunts playable threats and forks.
 */

#include "../heuristics.h"

float fork_hunter_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.3f * center_control(state, player);
    score += 1.0f * window_strength(state, player);
    score += 1.2f * playable_threats(state, player);
    score += 0.9f * double_threat_pressure(state, player);
    score += 0.6f * line_potential(state, player);

    return score;
}
