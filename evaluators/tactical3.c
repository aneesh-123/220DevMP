/*
 * Tactical v3: tuned to handle sharper tactical races more cleanly.
 * It leans a bit harder on urgent defense, fork pressure, and stable shape.
 */

#include "../heuristics.h"

float tactical3_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.6f * center_control(state, player);
    score += 1.05f * window_strength(state, player);
    score += 1.30f * playable_threats(state, player);
    score += 1.25f * immediate_loss_alarm(state, player);
    score += 1.05f * double_threat_pressure(state, player);
    score += 0.85f * line_potential(state, player);
    score += 0.45f * supported_connections(state, player);
    score += 0.15f * removal_availability_value(state, player);
    score += 0.10f * removal_fragility(state, player);

    return score;
}
