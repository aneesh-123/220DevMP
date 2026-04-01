#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "engine/board.h"

/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * Each function takes a game state and a player, and returns a float score.
 * Combine these in your evaluators to build a strong bot.
 *
 * ADD YOUR OWN HEURISTIC FUNCTIONS HERE.
 */

/* Returns a large bonus/penalty for terminal (win/loss) states, else 0.
 * Always include this — without it the bot won't recognise wins at leaves. */
float terminal_state_bonus(const GameState *state, int player);

/* Scores pieces by column centrality (col 3 = 3pts, cols 2&4 = 2pts, etc.).
 * Positive for player's pieces, negative for opponent's. */
float center_control(const GameState *state, int player);

/* Scans all windows of 4 cells and scores partial connections.
 * Rewards 2- and 3-in-a-row; penalises the same for the opponent.
 * Only counts windows with a playable (immediately reachable) empty cell. */
float count_threats(const GameState *state, int player);

/* Returns +1 if only the player has removal remaining,
 * -1 if only the opponent does, 0 if equal. */
float removal_advantage(const GameState *state, int player);

#endif
