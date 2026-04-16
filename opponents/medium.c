/*
 * Medium opponent: terminal bonus + window score + center control.
 *
 * Stronger than the weak opponent (which only uses center preference)
 * but lacks threat_count and removal_advantage.  This means it:
 *   - Understands 2-in-a-row and 3-in-a-row positional pressure
 *   - Prefers center columns
 *   - Does NOT explicitly recognise forks (two simultaneous threats)
 *   - Does NOT value having removal moves as a resource
 *
 * Use this as a mid-tier benchmark in experiments.  The final evaluator
 * should reliably beat medium because it identifies forks and uses
 * removal moves more purposefully.
 */

#include "../heuristics.h"

float medium_evaluate(const GameState *state, int player) {
    float score = terminal_state_bonus(state, player);
    if (state->is_terminal)
        return score;

    score += 4.0f * window_score(state, player);
    score += 1.5f * center_control(state, player);

    return score;
}
