#ifndef OPPONENTS_H
#define OPPONENTS_H

#include "../engine/board.h"
#include "../engine/search.h"
#include "../engine/bot.h"

/*
 * Opponent roster — each bot represents a distinct strategic personality.
 * This variety ensures that experiments exercise different weaknesses
 * rather than just testing raw strength levels.
 *
 * Tier / personality summary:
 *
 *   Weak             — center column preference only; no window reasoning.
 *                      Baseline: any reasonable evaluator should beat this.
 *
 *   Medium           — window_score + center_control; no threat/removal
 *                      awareness.  A solid positional bot without tactics.
 *
 *   Aggressive       — asymmetric windows: own 3-in-a-row weighted 3× higher
 *                      than blocking opponent's.  Builds threats fast but
 *                      lets the opponent complete lines.
 *
 *   Defensive        — inverted asymmetry: blocks opponent 3-in-a-rows 6×
 *                      more than it values its own.  Rarely lets you build
 *                      without pressure, but won't win on its own either.
 *
 *   RemovalAggressive — over-weights removals (15× vs balanced 3×); wastes
 *                      the removal on low-value targets and mis-times it.
 *
 *   ThreatOnly       — sees only immediate 3+1empty threats; completely
 *                      blind to positional development and forks.
 */

extern float weak_evaluate(const GameState *state, int player);
extern float medium_evaluate(const GameState *state, int player);
extern float aggressive_evaluate(const GameState *state, int player);
extern float defensive_evaluate(const GameState *state, int player);
extern float removal_aggressive_evaluate(const GameState *state, int player);
extern float threat_only_evaluate(const GameState *state, int player);

#define DEFAULT_DEPTH 4

static Bot opponent_bots[] = {
    { weak_evaluate,               DEFAULT_DEPTH, "Weak"             },
    { medium_evaluate,             DEFAULT_DEPTH, "Medium"           },
    { aggressive_evaluate,         DEFAULT_DEPTH, "Aggressive"       },
    { defensive_evaluate,          DEFAULT_DEPTH, "Defensive"        },
    { removal_aggressive_evaluate, DEFAULT_DEPTH, "RemovalAggressive"},
    { threat_only_evaluate,        DEFAULT_DEPTH, "ThreatOnly"       }
};

#define NUM_OPPONENTS (sizeof(opponent_bots) / sizeof(opponent_bots[0]))

#endif
