/*
 * V2: V1 + threat detection.
 *
 * Adds count_threats on top of V1: the bot now scores 2- and 3-in-a-row
 * patterns for itself and penalises the same for the opponent.
 * This should produce a clear win-rate jump over V1 because the bot can
 * now build threats and recognise when to block.
 */

#include "../heuristics.h"

float v2_threats_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.5f * center_control(state, player);
    score += 3.0f * count_threats(state, player);

    return score;
}
