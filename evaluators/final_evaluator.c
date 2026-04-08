/*
 * Strong evaluator combining all heuristics.
 * Uses threat detection, center control, and removal flexibility.
 */

#include "../heuristics.h"

float final_evaluator(const GameState *state, int player) {
    float score = 0.0f;

    /* Terminal state check: massive bonus/penalty for wins/losses */
    score += terminal_state_bonus(state, player);

    /* Threat detection: creating/blocking winning opportunities (high priority) */
    score += 9.0f * threat_detection(state, player);

    /* Center control: flexibility and multiple threat potential */
    // score += 3.0f * center_control(state, player);

    /* Removal flexibility: tactical advantage in this game variant */
    score += 2.0f * removal_flexibility(state, player);

    /* Piece advantage: simple material count */
    score += 0.5f * piece_advantage(state, player);

    return score;
}