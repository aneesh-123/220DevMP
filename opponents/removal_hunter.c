/*
 * Removal Hunter opponent: exploits the removal mechanic heavily.
 * Tries to maintain removal advantage and use removals to disrupt threats.
 */

#include "../heuristics.h"

float removal_hunter_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 6.0f * removal_flexibility(state, player);
    score += 2.0f * threat_detection(state, player);
    score += 0.5f * piece_advantage(state, player);

    return score;
}
