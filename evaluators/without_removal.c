/*
 * Ablation evaluator: strong_evaluate WITHOUT removal flexibility.
 * Used to test the impact of removal_flexibility heuristic.
 */

#include "../heuristics.h"

float without_removal_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 3.0f * threat_detection(state, player);
    score += 1.5f * center_control(state, player);
    /* Removal flexibility REMOVED */
    score += 0.5f * piece_advantage(state, player);

    return score;
}
