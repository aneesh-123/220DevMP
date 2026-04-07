/*
 * Ablation evaluator: strong_evaluate WITHOUT threat detection.
 * Used to test the impact of threat_detection heuristic.
 */

#include "../heuristics.h"

float without_threat_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    /* Threat detection REMOVED */
    score += 1.5f * center_control(state, player);
    score += 2.0f * removal_flexibility(state, player);
    score += 0.5f * piece_advantage(state, player);

    return score;
}
