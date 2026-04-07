/*
 * Medium opponent: terminal bonus + threat detection only.
 * Stronger than Weak (which uses only center preference).
 * Used to test robustness against a search-aware opponent.
 */

#include "../heuristics.h"

float medium_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 3.0f * threat_detection(state, player);

    return score;
}
