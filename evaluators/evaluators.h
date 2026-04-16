#ifndef EVALUATORS_H
#define EVALUATORS_H

#include "../engine/board.h"
#include "../engine/search.h"
#include "../engine/bot.h"

/*
 * Register your evaluators here.
 * To add a new evaluator:
 * 1. Create a new .c file in this folder
 * 2. Add an extern declaration below
 * 3. Add an entry to evaluator_bots[]
 * 4. Rebuild
 *
 * Ablation ladder (for report):
 *   window_evaluator  — terminal + window_score only
 *   center_evaluator  — + center_control
 *   final_evaluator   — + threat_count + removal_advantage + piece_count
 */

extern float final_evaluator(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot evaluator_bots[] = {
    { final_evaluator,  DEFAULT_DEPTH, "Final"        }
};

#define NUM_EVALUATORS (sizeof(evaluator_bots) / sizeof(evaluator_bots[0]))

#endif
