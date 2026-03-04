#include "ai_mode4.h"
#include "ai_mode3.h"
#include "eval.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Mode 4: Alpha-Beta + Move Ordering + TT + Iterative Deepening.
 *
 * Instead of jumping straight to the target depth, we search
 * depth 1, then 2, then 3, ... up to the requested depth.
 *
 * Why this helps:
 *   - Each shallower pass fills the transposition table with
 *     scores for positions. When the deeper pass encounters
 *     these positions, it can use the cached results.
 *   - The overhead of the shallow passes is small because they
 *     explore far fewer nodes (the tree grows exponentially).
 *   - In a time-limited setting (not implemented here), iterative
 *     deepening lets you always have a "best move so far" to fall
 *     back on if time runs out.
 *
 * This mode reuses Mode 3's search function via ai_choose_move_mode3.
 * The TT persists across iterations since mode3_tt_clear is NOT
 * called between depths — that's the whole point.
 *
 * Metrics (nodes_visited, tt_probes, tt_hits) accumulate across
 * all iterations so the final numbers reflect total work done.
 */

Move ai_choose_move_mode4(const GameState *state, int depth, Metrics *m) {
    Move best_move = {MOVE_DROP, 0, -1};

    /* Search from depth 1 up to the requested depth.
     * Each iteration uses Mode 3's search, which reads/writes
     * the same TT. Shallow iterations "warm up" the TT. */
    for (int d = 1; d <= depth; d++) {
        Metrics iter_m;
        metrics_reset(&iter_m, d);

        Move mv = ai_choose_move_mode3(state, d, &iter_m);

        /* Accumulate metrics from this iteration. */
        m->nodes_visited += iter_m.nodes_visited;
        m->tt_probes     += iter_m.tt_probes;
        m->tt_hits       += iter_m.tt_hits;

        /* Keep the move from the deepest completed search. */
        best_move = mv;
        m->depth_completed = d;
    }

    return best_move;
}
