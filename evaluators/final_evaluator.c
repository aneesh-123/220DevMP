/*
 * Strong evaluator combining all currently available heuristics.
 */

#include "../heuristics.h"

float final_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.0f * count_threats(state, player);
    score += 1.5f * center_control(state, player);
    score += 8.0f * removal_advantage(state, player);

    return score;
}
