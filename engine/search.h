#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "moves.h"

/* Evaluator function pointer type.
 * Takes a game state and the player to evaluate for.
 * Returns a float score (higher = better for that player). */
typedef float (*EvaluateFn)(const GameState *state, int player);

typedef struct {
    Move best_move;
    float score;
    int depth_searched;
    int has_move; /* 1 if best_move is valid, 0 if no moves */
} SearchResult;

/* Run minimax search from the given state.
 * depth: how many plies to search.
 * evaluate: the evaluation function to use at leaf nodes. */
SearchResult search(const GameState *state, int depth, EvaluateFn evaluate);

#endif
