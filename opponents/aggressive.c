/*
 * Aggressive opponent: builds own threats and piece advantage, never blocks.
 * Purely offensive — ignores opponent 3-in-a-rows entirely.
 */

#include "../heuristics.h"

float aggressive_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 5.0f * offensive_threats(state, player);
    score += 1.5f * piece_advantage(state, player);

    return score;
}
