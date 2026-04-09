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
extern float terminal_only_evaluate(const GameState *state, int player);
extern float piece_count_evaluate(const GameState *state, int player);
extern float threat_seeker_evaluate(const GameState *state, int player);
extern float defensive_evaluate(const GameState *state, int player);
extern float removal_aware_evaluate(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot opponent_bots[] = {
    { terminal_only_evaluate,  DEFAULT_DEPTH, "TerminalOnly"  },
    { piece_count_evaluate,    DEFAULT_DEPTH, "PieceCount"    },
    { weak_evaluate,           DEFAULT_DEPTH, "Weak"          },
    { threat_seeker_evaluate,  DEFAULT_DEPTH, "ThreatSeeker"  },
    { defensive_evaluate,      DEFAULT_DEPTH, "Defensive"     },
    { removal_aware_evaluate,  DEFAULT_DEPTH, "RemovalAware"  },
};

#define NUM_OPPONENTS (sizeof(opponent_bots) / sizeof(opponent_bots[0]))

#endif
