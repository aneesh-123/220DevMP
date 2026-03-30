#ifndef OPPONENTS_H
#define OPPONENTS_H

#include "../engine/board.h"
#include "../engine/search.h"
#include "../engine/bot.h"

/*
 * Declare your opponent evaluate functions here, then add them to the arrays below.
 * To add a new opponent:
 * 1. Create a new .c file in this folder
 * 2. Add an extern declaration below
 * 3. Add an entry to opponent_bots[]
 * 4. Rebuild with: make
 */

extern float weak_evaluate(const GameState *state, int player);
extern float center_only_evaluate(const GameState *state, int player);
extern float greedy_windows_evaluate(const GameState *state, int player);
extern float defensive_windows_evaluate(const GameState *state, int player);
extern float removal_blind_evaluate(const GameState *state, int player);
extern float fork_hunter_evaluate(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot opponent_bots[] = {
    { weak_evaluate, DEFAULT_DEPTH, "Weak" },
    { center_only_evaluate, DEFAULT_DEPTH, "CenterOnly" },
    { greedy_windows_evaluate, DEFAULT_DEPTH, "GreedyWindows" },
    { defensive_windows_evaluate, DEFAULT_DEPTH, "DefensiveWindows" },
    { removal_blind_evaluate, DEFAULT_DEPTH, "RemovalBlind" },
    { fork_hunter_evaluate, DEFAULT_DEPTH, "ForkHunter" },
};

#define NUM_OPPONENTS (sizeof(opponent_bots) / sizeof(opponent_bots[0]))

#endif
