/*
 * Tactical evaluator: adds real Connect 4 pressure recognition.
 */

#include "../heuristics.h"

float tactical_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.8f * center_control(state, player);
    score += 1.0f * window_strength(state, player);
    score += 1.2f * playable_threats(state, player);
    score += 1.0f * immediate_loss_alarm(state, player);
    score += 0.8f * double_threat_pressure(state, player);
    score += 0.7f * line_potential(state, player);

    return score;
}
