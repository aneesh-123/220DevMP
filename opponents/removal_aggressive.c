/*
 * Removal-aggressive opponent: over-values removal moves.
 *
 * Uses window_score for positional reasoning but gives an outsized
 * weight to having more removals than the opponent (15x vs the
 * balanced 3x in final_evaluator).  This causes it to spend its
 * removal move early and in low-value situations, and to over-rate
 * positions where it still holds a removal even when no good target
 * exists.
 *
 * Consequence: wastes removals on marginal disruptions; struggles
 * in positions where careful, timed removal use is critical.
 *
 * Tests whether your evaluator correctly times removal moves and
 * does not over- or under-value them.
 */

#include "../heuristics.h"

float removal_aggressive_evaluate(const GameState *state, int player) {
    float score = terminal_state_bonus(state, player);
    if (state->is_terminal)
        return score;

    /* Standard positional base */
    score += 4.0f  * window_score(state, player);
    score += 1.5f  * center_control(state, player);

    /* Drastically over-values the removal resource */
    score += 15.0f * removal_advantage(state, player);

    return score;
}
