/*
 * Removal-aware evaluator: extends tactical play for the removal variant.
 */

#include "../heuristics.h"

float removal_smart_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.7f * center_control(state, player);
    score += 1.05f * window_strength(state, player);
    score += 1.25f * playable_threats(state, player);
    score += 1.1f * immediate_loss_alarm(state, player);
    score += 0.9f * double_threat_pressure(state, player);
    score += 0.8f * line_potential(state, player);
    score += 0.35f * supported_connections(state, player);
    score += 0.45f * removal_availability_value(state, player);
    score += 0.35f * removal_fragility(state, player);

    return score;
}
