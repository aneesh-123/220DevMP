#ifndef EVALUATORS_H
#define EVALUATORS_H

#include "../engine/board.h"
#include "../engine/search.h"
#include "../engine/bot.h"

/*
 * Register your evaluators here.
 * To add a new evaluator:
 * 1. Create a new .c file in this folder (copy empty.c as a starting point)
 * 2. Add an extern declaration below
 * 3. Add an entry to evaluator_bots[]
 * 4. Rebuild with: make
 */

extern float empty_evaluate(const GameState *state, int player);
extern float baseline_evaluate(const GameState *state, int player);
extern float tactical_evaluate(const GameState *state, int player);
extern float tactical2_evaluate(const GameState *state, int player);
extern float tactical3_evaluate(const GameState *state, int player);
extern float tactical4_evaluate(const GameState *state, int player);
extern float tactical5_evaluate(const GameState *state, int player);
extern float tactical6_evaluate(const GameState *state, int player);
extern float removal_smart_evaluate(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot evaluator_bots[] = {
    { empty_evaluate, DEFAULT_DEPTH, "Empty" },
    { baseline_evaluate, DEFAULT_DEPTH, "Baseline" },
    { tactical_evaluate, DEFAULT_DEPTH, "Tactical" },
    { tactical2_evaluate, DEFAULT_DEPTH, "Tactical2" },
    { tactical3_evaluate, DEFAULT_DEPTH, "Tactical3" },
    { tactical4_evaluate, DEFAULT_DEPTH, "Tactical4" },
    { tactical5_evaluate, DEFAULT_DEPTH, "Tactical5" },
    { tactical6_evaluate, DEFAULT_DEPTH, "Tactical6" },
    { removal_smart_evaluate, DEFAULT_DEPTH, "RemovalSmart" },
};

#define NUM_EVALUATORS (sizeof(evaluator_bots) / sizeof(evaluator_bots[0]))

#endif
