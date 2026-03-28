/*
 * Starter evaluator — baseline that only checks terminal states.
 *
 * Copy this file to create a new evaluator, then add your heuristics.
 */

#include "../heuristics.h"

float empty_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);

    /* Add your heuristics here, e.g.:
     * score += 2.0f * center_control(state, player);
     * score += 5.0f * threat_detection(state, player);
     */

    return score;
}
