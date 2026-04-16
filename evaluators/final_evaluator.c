/*
 * Final evaluator: combines all heuristics with tuned weights.
 *
 * Weight rationale:
 *   terminal_state_bonus : dominates all other terms (win/loss = ±10000)
 *   window_score * 4.0   : core positional reasoning — values piece
 *                          connectivity and threats across all 69 windows
 *   threat_count * 20.0  : amplifies immediate (3+1empty) threats beyond
 *                          what window_score alone signals; heavily rewards
 *                          forks (2+ simultaneous threats = near-certain win)
 *   center_control * 1.5 : tiebreaker — center pieces participate in more
 *                          windows so they're inherently more valuable
 *   removal_advantage*3.0: having an extra removal is a concrete tactical
 *                          resource; can disrupt threats or create wins via
 *                          gravity
 *   piece_count * 0.3    : very small nudge toward having more pieces on the
 *                          board; mostly a tiebreaker in equal positions
 */

#include "../heuristics.h"

float final_evaluator(const GameState *state, int player) {
    float score = 0.0f;

    /* Terminal check — must be first and returned immediately */
    score = terminal_state_bonus(state, player);
    if (state->is_terminal)
        return score;

    score += 10.0f  * window_score(state, player);
    score += 20.0f * threat_count(state, player);
    score += 1.5f  * center_control(state, player);
    score += 3.0f  * removal_advantage(state, player);
    score += 0.3f  * piece_count(state, player);

    return score;
}
