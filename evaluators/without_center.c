/*
 * Ablation evaluator: strong_evaluate WITHOUT center control.
 * Used to test the impact of center_control heuristic.
 */

#include "../heuristics.h"

float without_center_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 3.0f * threat_detection(state, player);
    /* Center control REMOVED */
    score += 2.0f * removal_flexibility(state, player);
    score += 0.5f * piece_advantage(state, player);

    return score;
}
