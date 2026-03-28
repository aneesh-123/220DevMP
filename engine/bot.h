#ifndef BOT_H
#define BOT_H

#include "board.h"
#include "moves.h"
#include "search.h"

typedef struct {
    EvaluateFn evaluate;
    int depth;
    const char *name;
} Bot;

/* Create a bot with the given evaluator, depth, and display name. */
Bot bot_create(EvaluateFn evaluate, int depth, const char *name);

/* Choose the best move for the current state. Returns 1 if a move was found. */
int bot_choose_move(const Bot *bot, const GameState *state, Move *move_out);

#endif
