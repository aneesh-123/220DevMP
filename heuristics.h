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

/* Returns a large bonus/penalty for terminal (win/loss) states, else 0. */
float terminal_state_bonus(const GameState *state, int player);

/* Returns player pieces minus opponent pieces. */
float piece_advantage(const GameState *state, int player);

/* Returns score based on piece proximity to center column. */
float center_control(const GameState *state, int player);

/* Returns score based on open 3-in-a-row threats for/against player. */
float threat_detection(const GameState *state, int player);

/* Returns score for player's own open 3-in-a-row threats only (no blocking term). */
float offensive_threats(const GameState *state, int player);

/* Returns score for opponent's open 3-in-a-row threats only (blocking term only). */
float defensive_blocks(const GameState *state, int player);

/* Returns score based on remaining removal moves relative to opponent. */
float removal_flexibility(const GameState *state, int player);

#endif
