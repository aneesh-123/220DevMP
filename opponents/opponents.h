#ifndef OPPONENTS_H
#define OPPONENTS_H

#include "../engine/board.h"
#include "../engine/search.h"
#include "../engine/bot.h"

/*
 * Opponent suite for benchmarking and stress-testing student evaluators.
 *
 * Organized into four tiers:
 *   Tier 1 (Weak)         - pure_positional, passive_builder, edge_presser
 *   Tier 2 (Intermediate) - threat_reactor, vertical_stacker, fork_hunter, removal_panicker
 *   Tier 3 (Strong)       - tactical_aggressor, fortress_defender, removal_tactician, diagonal_specialist
 *   Tier 4 (Adversarial)  - trap_setter, removal_baiter, endgame_closer, anti_shallow
 *
 * To add a new opponent:
 * 1. Create a new .c file in this folder
 * 2. Add an extern declaration below
 * 3. Add an entry to opponent_bots[]
 * 4. Rebuild with: make
 */

/* Tier 1: Weak / Baseline */
extern float pure_positional_evaluate(const GameState *state, int player);
extern float passive_builder_evaluate(const GameState *state, int player);
extern float edge_presser_evaluate(const GameState *state, int player);

/* Tier 2: Intermediate */
extern float threat_reactor_evaluate(const GameState *state, int player);
extern float vertical_stacker_evaluate(const GameState *state, int player);
extern float fork_hunter_evaluate(const GameState *state, int player);
extern float removal_panicker_evaluate(const GameState *state, int player);

/* Tier 3: Strong */
extern float tactical_aggressor_evaluate(const GameState *state, int player);
extern float fortress_defender_evaluate(const GameState *state, int player);
extern float removal_tactician_evaluate(const GameState *state, int player);
extern float diagonal_specialist_evaluate(const GameState *state, int player);

/* Tier 4: Adversarial / Specialist */
extern float trap_setter_evaluate(const GameState *state, int player);
extern float removal_baiter_evaluate(const GameState *state, int player);
extern float endgame_closer_evaluate(const GameState *state, int player);
extern float anti_shallow_evaluate(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot opponent_bots[] = {
    /* Tier 1 */
    { pure_positional_evaluate,  DEFAULT_DEPTH, "PurePositional"   },
    { passive_builder_evaluate,  DEFAULT_DEPTH, "PassiveBuilder"   },
    { edge_presser_evaluate,     DEFAULT_DEPTH, "EdgePresser"      },
    /* Tier 2 */
    { threat_reactor_evaluate,   DEFAULT_DEPTH, "ThreatReactor"    },
    { vertical_stacker_evaluate, DEFAULT_DEPTH, "VerticalStacker"  },
    { fork_hunter_evaluate,      DEFAULT_DEPTH, "ForkHunter"       },
    { removal_panicker_evaluate, DEFAULT_DEPTH, "RemovalPanicker"  },
    /* Tier 3 */
    { tactical_aggressor_evaluate,  DEFAULT_DEPTH, "TacticalAggressor"  },
    { fortress_defender_evaluate,   DEFAULT_DEPTH, "FortressDefender"   },
    { removal_tactician_evaluate,   DEFAULT_DEPTH, "RemovalTactician"   },
    { diagonal_specialist_evaluate, DEFAULT_DEPTH, "DiagonalSpecialist" },
    /* Tier 4 */
    { trap_setter_evaluate,    DEFAULT_DEPTH, "TrapSetter"    },
    { removal_baiter_evaluate, DEFAULT_DEPTH, "RemovalBaiter" },
    { endgame_closer_evaluate, DEFAULT_DEPTH, "EndgameCloser" },
    { anti_shallow_evaluate,   DEFAULT_DEPTH, "AntiShallow"   },
};

#define NUM_OPPONENTS (sizeof(opponent_bots) / sizeof(opponent_bots[0]))

#endif
