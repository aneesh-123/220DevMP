#include "../heuristics.h"

/*
 * Final evaluator — combines all heuristics with tuned weights.
 *
 * Weight rationale (highest to lowest priority):
 *
 *  terminal_state_bonus   — must dominate everything; win/loss is absolute.
 *
 *  threat_count * 50      — actionable threats are the most important mid-game
 *                           signal. Each threat the opponent cannot answer is
 *                           worth ~50 points.
 *
 *  double_threat_bonus    — already scaled to ±500 inside the function.
 *                           A fork (2+ simultaneous threats) is nearly a win
 *                           at low search depths.
 *
 *  open_window_score * 3  — measures long-term positional potential (includes
 *                           non-immediately-playable windows). Internally
 *                           weights 3-windows at 10× vs 2-windows.
 *
 *  removal_flexibility*10 — each removal is a wildcard that can reshape the
 *                           board; the asymmetry (having removals while the
 *                           opponent does not) is a significant edge.
 *
 *  removal_threat_exposure*5 — penalizes having critical pieces that the
 *                           opponent can remove to collapse our threats.
 *
 *  height_penalty * 3     — pieces in the top two rows have fewer lines and
 *                           are easy removal targets.
 *
 *  center_control * 2     — small but consistent positional value; center
 *                           pieces participate in more winning lines.
 *
 *  gravity_chain_potential*2 — rewards positions where we can use a removal
 *                           to drop our piece into a stronger square.
 */
float final_evaluator(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);

    /* Short-circuit: no need to evaluate a finished game further. */
    if (state->is_terminal)
        return score;

    score += 50.0f * threat_count(state, player);
    score +=  1.0f * double_threat_bonus(state, player);
    score +=  3.0f * open_window_score(state, player);
    score += 15.0f * blocking_priority(state, player);   /* asymmetric: opp 3-windows penalized 2x at 15 = 30 effective */
    score += 10.0f * removal_flexibility(state, player);
    // score +=  5.0f * removal_threat_exposure(state, player);
    // score +=  3.0f * height_penalty(state, player);
    score +=  2.0f * center_control(state, player);
    // score +=  2.0f * gravity_chain_potential(state, player);

    return score;
}
