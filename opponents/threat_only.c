/*
 * Threat-only opponent: purely reactive, no positional reasoning.
 *
 * Only evaluates immediate threats (3-of-one-player + 1 empty window)
 * and terminal states.  Has no awareness of:
 *   - 1-in-a-row or 2-in-a-row development
 *   - Center column value
 *   - Removal resources
 *
 * Consequence: completely reactive — responds to the most pressing
 * immediate threat but cannot plan ahead or build positions.  Easily
 * defeated by a bot that sets up a fork (two threats at once), because
 * threat_only sees both but cannot distinguish "this move creates two
 * threats simultaneously" from "this move creates one".
 *
 * Tests that your evaluator can proactively build multi-threat
 * positions rather than waiting for obvious 3-in-a-rows to appear.
 */

#include "../heuristics.h"

float threat_only_evaluate(const GameState *state, int player) {
    float score = terminal_state_bonus(state, player);
    if (state->is_terminal)
        return score;

    /* Only immediate threats — no long-range positional value */
    score += 50.0f * threat_count(state, player);

    return score;
}
