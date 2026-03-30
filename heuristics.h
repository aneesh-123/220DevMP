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
float center_control(const GameState *state, int player);
float window_strength(const GameState *state, int player);
float playable_threats(const GameState *state, int player);
float immediate_loss_alarm(const GameState *state, int player);
float double_threat_pressure(const GameState *state, int player);
float line_potential(const GameState *state, int player);
float supported_connections(const GameState *state, int player);
float forced_block_value(const GameState *state, int player);
float unstable_threat_penalty(const GameState *state, int player);
float immediate_removal_win_swing(const GameState *state, int player);
float removal_availability_value(const GameState *state, int player);
float removal_fragility(const GameState *state, int player);

#endif
