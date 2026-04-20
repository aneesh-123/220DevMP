#ifndef RULES_H
#define RULES_H

#include "board.h"
#include "moves.h"

int is_legal_move(const GameState *state, Move move);
void apply_move(GameState *state, Move move);
int check_win(const GameState *state, int player);
int is_board_full(const GameState *state);

int column_height(const GameState *state, int col);

#endif
