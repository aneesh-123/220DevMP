/*
 * Tactical v2: keeps the tactical core but adds shape quality and
 * light removal awareness so the evaluator is less blind in this variant.
 */

#include "../heuristics.h"

float tactical2_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.7f * center_control(state, player);
    score += 1.05f * window_strength(state, player);
    score += 1.25f * playable_threats(state, player);
    score += 1.1f * immediate_loss_alarm(state, player);
    score += 0.9f * double_threat_pressure(state, player);
    score += 0.8f * line_potential(state, player);
    score += 0.35f * supported_connections(state, player);
    score += 0.20f * removal_availability_value(state, player);
    score += 0.15f * removal_fragility(state, player);

    return score;
}
