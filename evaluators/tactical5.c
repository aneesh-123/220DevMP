/*
 * Tactical v5: adds explicit one-removal win detection so it avoids
 * positions where the opponent can instantly win by pulling a piece.
 */

#include "../heuristics.h"

float tactical5_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.55f * center_control(state, player);
    score += 1.05f * window_strength(state, player);
    score += 1.30f * playable_threats(state, player);
    score += 1.30f * immediate_loss_alarm(state, player);
    score += 1.10f * double_threat_pressure(state, player);
    score += 0.90f * line_potential(state, player);
    score += 0.45f * supported_connections(state, player);
    score += 0.75f * forced_block_value(state, player);
    score += 0.80f * unstable_threat_penalty(state, player);
    score += 1.25f * immediate_removal_win_swing(state, player);
    score += 0.10f * removal_availability_value(state, player);
    score += 0.15f * removal_fragility(state, player);

    return score;
}
