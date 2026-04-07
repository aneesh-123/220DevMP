/*
 * Defensive opponent: prioritizes blocking opponent threats over creating its own.
 * Reacts to what the opponent is doing rather than building proactively.
 */

#include "../heuristics.h"

float defensive_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 5.0f * defensive_blocks(state, player);
    score += 1.0f * offensive_threats(state, player);
    score += 0.5f * center_control(state, player);

    return score;
}
