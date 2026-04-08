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
 * 4. Rebuild the project
 */

extern float empty_evaluate(const GameState *state, int player);
extern float strong_evaluate(const GameState *state, int player);
extern float without_threat_evaluate(const GameState *state, int player);
extern float without_center_evaluate(const GameState *state, int player);
extern float without_removal_evaluate(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot evaluator_bots[] = {
    { empty_evaluate,           DEFAULT_DEPTH, "Empty"           },
    { strong_evaluate,          DEFAULT_DEPTH, "Strong"          },
    { without_threat_evaluate,  DEFAULT_DEPTH, "Without Threat"  },
    { without_center_evaluate,  DEFAULT_DEPTH, "Without Center"  },
    { without_removal_evaluate, DEFAULT_DEPTH, "Without Removal" },
};

#define NUM_EVALUATORS (sizeof(evaluator_bots) / sizeof(evaluator_bots[0]))

#endif
