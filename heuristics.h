#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "engine/board.h"

/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * Each function takes a game state and a player, and returns a float score.
 * Positive = good for player, negative = bad for player.
 * Combine these in your evaluators to build a strong bot.
 */

/* Returns a large bonus/penalty for terminal (win/loss) states, else 0.
 * Always call this first in your evaluator and return early if non-zero. */
float terminal_state_bonus(const GameState *state, int player);

/* Piece count differential: player_pieces - opponent_pieces.
 * Simple baseline — more pieces generally means more influence. */
float piece_count(const GameState *state, int player);

/* Center column control: pieces weighted by proximity to col 3.
 * Weights: col 0=1, 1=2, 2=3, 3=4, 4=3, 5=2, 6=1.
 * Center pieces participate in more potential 4-windows. */
float center_control(const GameState *state, int player);

/* Window score: evaluates all 4-cell windows (horizontal, vertical,
 * both diagonals). Scores each window based on how many of each
 * player's pieces it contains. Blocked windows (both players present)
 * score 0. Key scores: 3-in-a-row = ±50, 2-in-a-row = ±5, 1 = ±1.
 * This is the core positional heuristic. */
float window_score(const GameState *state, int player);

/* Threat count: counts windows with exactly 3 of one player's pieces
 * and 1 empty cell (immediate winning threats). Returns
 * player_threats - opponent_threats. Two simultaneous threats
 * (a "fork") guarantee a win since only one can be blocked. */
float threat_count(const GameState *state, int player);

/* Removal advantage: removals_remaining[player] - removals_remaining[opponent].
 * Having more removal moves provides tactical flexibility — you can
 * disrupt opponent threats or create winning positions via gravity. */
float removal_advantage(const GameState *state, int player);

#endif
