#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "engine/board.h"

/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * Each function takes a game state and the player to evaluate for, and
 * returns a float score where higher is better for `player`.
 *
 * Combine these in your evaluators to build a strong bot.
 */

/* Returns a large bonus/penalty for terminal (win/loss) states, else 0. */
float terminal_state_bonus(const GameState *state, int player);

/*
 * Score based on all open windows of 4 cells.
 * A window is "open" if it contains N player pieces and 0 opponent pieces.
 * Weights 3-in-a-row windows much higher than 2-in-a-row.
 * Returns (own score - opponent score).
 */
float open_window_score(const GameState *state, int player);

/*
 * Asymmetric blocking bonus: penalizes raw opponent 3-in-a-row windows
 * (no gravity filter) more heavily than it rewards own 3-in-a-row windows.
 * Use this to out-block opponents that heavily weight threat disruption.
 * Returns (own_3_windows * 1) - (opp_3_windows * multiplier) where
 * multiplier > 1 encodes the blocking preference.
 */
float blocking_priority(const GameState *state, int player);

/*
 * Count immediately actionable threats: 3-in-a-row windows where the
 * single empty cell is gravity-reachable (i.e. the top of its column).
 * These are threats that can be played or must be blocked this turn.
 * Returns (own threats - opponent threats).
 */
float threat_count(const GameState *state, int player);

/*
 * Large bonus if the player holds 2 or more simultaneous actionable threats
 * (a "fork"), and a large penalty if the opponent does. One threat can be
 * blocked; two cannot. Returns +500 for own fork, -500 for opponent fork.
 */
float double_threat_bonus(const GameState *state, int player);

/*
 * Weight pieces by column distance from the center (col 3).
 * Center pieces participate in more potential 4-in-a-row lines.
 * Returns (own center score - opponent center score).
 */
float center_control(const GameState *state, int player);

/*
 * Penalize pieces stacked in the top two rows (rows 0-1).
 * High pieces have fewer lines through them and are easy removal targets.
 * Returns (opponent high pieces - own high pieces), so adding this to a
 * score penalizes having pieces near the top.
 */
float height_penalty(const GameState *state, int player);

/*
 * Reward having more removal moves remaining than the opponent.
 * Removals are a strategic resource unique to this variant; having them
 * while the opponent doesn't is a meaningful asymmetric advantage.
 * Returns (own removals - opponent removals).
 */
float removal_flexibility(const GameState *state, int player);

/*
 * Penalize pieces that anchor open 3-in-a-row windows AND can be removed
 * by the opponent (i.e., opponent still has removals remaining).
 * A piece is more "exposed" the more open windows it participates in.
 * Returns 0 if opponent has no removals; otherwise returns a negative value.
 */
float removal_threat_exposure(const GameState *state, int player);

/*
 * Score positions where a player's piece sits directly above an opponent
 * piece in the same column. If that opponent piece is removed, gravity
 * drops the player's piece one row, potentially creating new lines.
 * Returns (own chain positions - opponent chain positions).
 */
float gravity_chain_potential(const GameState *state, int player);

#endif
