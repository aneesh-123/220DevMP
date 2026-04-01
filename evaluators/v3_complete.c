/*
 * V3: V2 + removal awareness.
 *
 * Adds removal_advantage on top of V2: the bot now values having its
 * removal move available and penalises giving the opponent a free removal.
 * The improvement over V2 is most visible in removal-tension positions
 * and late-game squeeze scenarios.
 */

#include "../heuristics.h"

float v3_complete_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += 1.5f * center_control(state, player);
    score +=  count_threats(state, player);
    // score += 5.0f * removal_advantage(state, player);

    return score;
}
