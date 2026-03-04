#include "ai_mode5.h"
#include "eval.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Mode 5: Gold Standard — every optimization combined.
 *
 * 1. Iterative deepening: search depth 1, 2, ... up to target.
 *    Each pass warms the TT for deeper searches.
 *
 * 2. Aspiration windows: after the first iteration, use a narrow
 *    window around the previous score. If the search fails high
 *    or low, re-search with a wider window.
 *
 * 3. Transposition table with best move: each TT entry stores the
 *    best move found. During move ordering, the TT move is tried
 *    first — this is the single most impactful optimization because
 *    iterative deepening ensures a good TT move exists.
 *
 * 4. Principal Variation Search (PVS): the first child (expected
 *    best move) is searched with a full window. All other children
 *    are searched with a zero/null window (alpha, alpha+1). If the
 *    null-window search finds a better score, a full re-search is
 *    done. This saves work because most non-PV nodes fail low.
 *
 * 5. Killer moves: two moves per ply that caused beta cutoffs in
 *    sibling nodes. These are tried early (after TT move, before
 *    normal moves) because a move that refutes one sibling often
 *    refutes others.
 *
 * 6. History heuristic: a global table counting how often each
 *    move causes a cutoff. Moves with high history scores are
 *    ordered earlier. Updated on every beta cutoff.
 */

/* ---- Zobrist hashing (independent from Mode 3) ---- */

static uint64_t z_pieces[ROWS][COLS][2];
static uint64_t z_turn;
static uint64_t z_rem[2][MAX_REMOVALS + 1];

static uint64_t z_rng = 0xFEDCBA9876543210ULL;  /* different seed from Mode 3 */

static uint64_t z_next(void) {
    z_rng ^= z_rng >> 12;
    z_rng ^= z_rng << 25;
    z_rng ^= z_rng >> 27;
    return z_rng * 0x2545F4914F6CDD1DULL;
}

void mode5_init(void) {
    z_rng = 0xFEDCBA9876543210ULL;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            for (int p = 0; p < 2; p++)
                z_pieces[r][c][p] = z_next();
    z_turn = z_next();
    for (int p = 0; p < 2; p++)
        for (int k = 0; k <= MAX_REMOVALS; k++)
            z_rem[p][k] = z_next();
}

static uint64_t hash_state(const GameState *s) {
    uint64_t h = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            if (s->board[r][c] == PLAYER_X) h ^= z_pieces[r][c][0];
            else if (s->board[r][c] == PLAYER_O) h ^= z_pieces[r][c][1];
        }
    if (s->turn == PLAYER_O) h ^= z_turn;
    h ^= z_rem[0][s->rem_x];
    h ^= z_rem[1][s->rem_o];
    return h;
}

/* ---- Transposition table with best-move storage ---- */

#define TT5_SIZE (1 << 20)
#define TT5_MASK (TT5_SIZE - 1)

#define BOUND_EXACT 0
#define BOUND_LOWER 1
#define BOUND_UPPER 2

typedef struct {
    uint64_t key;
    int      depth;
    int      score;
    int      bound;
    Move     best_move;  /* best move from this position */
    int      valid;
} TT5Entry;

static TT5Entry tt5[TT5_SIZE];

void mode5_tt_clear(void) {
    memset(tt5, 0, sizeof(tt5));
}

static void tt5_store(uint64_t key, int depth, int score, int bound,
                      const Move *best) {
    int idx = (int)(key & TT5_MASK);
    TT5Entry *e = &tt5[idx];
    /* Depth-preferred replacement: only overwrite if new depth >= stored. */
    if (e->valid && e->key == key && e->depth > depth)
        return;
    e->key       = key;
    e->depth     = depth;
    e->score     = score;
    e->bound     = bound;
    e->best_move = *best;
    e->valid     = 1;
}

/* Returns 1 if a usable score was found. Always fills best_move_out if entry exists. */
static int tt5_probe(uint64_t key, int depth, int alpha, int beta,
                     int *out_score, Move *best_move_out, int *has_move,
                     Metrics *m) {
    m->tt_probes++;
    int idx = (int)(key & TT5_MASK);
    TT5Entry *e = &tt5[idx];
    if (!e->valid || e->key != key) {
        *has_move = 0;
        return 0;
    }

    /* Always provide the stored best move for move ordering. */
    *best_move_out = e->best_move;
    *has_move = 1;

    if (e->depth < depth)
        return 0;  /* entry too shallow for a score cutoff, but move is still useful */

    m->tt_hits++;

    if (e->bound == BOUND_EXACT) {
        *out_score = e->score;
        return 1;
    }
    if (e->bound == BOUND_LOWER && e->score >= beta) {
        *out_score = e->score;
        return 1;
    }
    if (e->bound == BOUND_UPPER && e->score <= alpha) {
        *out_score = e->score;
        return 1;
    }
    return 0;
}

/* ---- Killer moves ---- */

#define MAX_PLY 64
static Move killers[MAX_PLY][2];

static void killers_clear(void) {
    memset(killers, 0, sizeof(killers));
}

static int moves_equal(const Move *a, const Move *b) {
    return a->type == b->type && a->col == b->col && a->row == b->row;
}

static void store_killer(int ply, const Move *m) {
    if (ply >= MAX_PLY) return;
    if (moves_equal(&killers[ply][0], m)) return;
    killers[ply][1] = killers[ply][0];
    killers[ply][0] = *m;
}

/* ---- History heuristic ---- */

/* history[move_type][col][row] — indexed by move characteristics. */
static int history[2][COLS][ROWS + 1];

static void history_clear(void) {
    memset(history, 0, sizeof(history));
}

static void history_update(const Move *m, int depth) {
    int r = (m->type == MOVE_DROP) ? ROWS : m->row;
    history[m->type][m->col][r] += depth * depth;  /* deeper cutoffs count more */
}

static int history_score(const Move *m) {
    int r = (m->type == MOVE_DROP) ? ROWS : m->row;
    return history[m->type][m->col][r];
}

/* ---- Move ordering ---- */

/*
 * Priority system (lower = tried first):
 *   0 = immediate win
 *   1 = TT best move (from a previous search)
 *   2 = killer move
 *   3+ = other moves, sorted by history + positional heuristic
 */
static int move_priority_m5(const GameState *s, const Move *mv,
                            const Move *tt_move, int has_tt_move,
                            int ply) {
    /* TT best move — try it first (after immediate wins). */
    if (has_tt_move && moves_equal(mv, tt_move))
        return 1;

    /* Killer moves. */
    if (ply < MAX_PLY) {
        if (moves_equal(mv, &killers[ply][0])) return 2;
        if (moves_equal(mv, &killers[ply][1])) return 2;
    }

    /* Immediate win check. */
    GameState copy;
    state_copy(&copy, s);
    if (mv->type == MOVE_DROP) apply_drop(&copy, mv->col);
    else                       apply_remove(&copy, mv->col, mv->row);
    if (check_winner(&copy) == s->turn) return 0;

    /* History-based + positional: lower value = higher priority.
     * Subtract history score so high-history moves get lower priority numbers. */
    int base;
    if (mv->type == MOVE_DROP)
        base = 100 + abs(mv->col - 3);  /* center-biased drops */
    else
        base = 200;  /* removals lower priority than drops */

    return base - (history_score(mv) / 100);
}

static void order_moves_m5(const GameState *s, Move *moves, int count,
                           const Move *tt_move, int has_tt_move, int ply) {
    int pri[MAX_MOVES];
    for (int i = 0; i < count; i++)
        pri[i] = move_priority_m5(s, &moves[i], tt_move, has_tt_move, ply);

    /* Insertion sort by priority (ascending = best first). */
    for (int i = 1; i < count; i++) {
        Move mt = moves[i]; int pt = pri[i]; int j = i - 1;
        while (j >= 0 && pri[j] > pt) {
            moves[j+1] = moves[j]; pri[j+1] = pri[j]; j--;
        }
        moves[j+1] = mt; pri[j+1] = pt;
    }
}

/* ---- PVS Alpha-Beta Search ---- */

static int pvs_search(const GameState *s, int depth, int ply,
                      int alpha, int beta, Move *best_out, Metrics *m) {
    m->nodes_visited++;

    /* Terminal checks. */
    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_board_full(s))   return 0;
    if (depth == 0)         return evaluate(s);

    /* TT probe. */
    uint64_t hash = hash_state(s);
    int cached;
    Move tt_move = {MOVE_DROP, 0, -1};
    int has_tt_move = 0;
    if (tt5_probe(hash, depth, alpha, beta, &cached, &tt_move, &has_tt_move, m)) {
        *best_out = tt_move;
        return cached;
    }

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    order_moves_m5(s, moves, count, &tt_move, has_tt_move, ply);

    int maximizing = (s->turn == PLAYER_X);
    int orig_alpha = alpha;
    int orig_beta  = beta;
    int best;
    Move best_move = moves[0];

    if (maximizing) {
        best = -SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child;
            state_copy(&child, s);
            if (moves[i].type == MOVE_DROP) apply_drop(&child, moves[i].col);
            else apply_remove(&child, moves[i].col, moves[i].row);
            switch_turn(&child);

            int val;
            Move child_best;
            if (i == 0) {
                /* First move (PV node): full window search. */
                val = pvs_search(&child, depth - 1, ply + 1,
                                 alpha, beta, &child_best, m);
            } else {
                /* Non-PV: null-window search. */
                val = pvs_search(&child, depth - 1, ply + 1,
                                 alpha, alpha + 1, &child_best, m);
                if (val > alpha && val < beta) {
                    /* Null-window failed high: re-search with full window. */
                    val = pvs_search(&child, depth - 1, ply + 1,
                                     alpha, beta, &child_best, m);
                }
            }

            if (val > best) { best = val; best_move = moves[i]; }
            if (best > alpha) alpha = best;
            if (alpha >= beta) {
                /* Beta cutoff — update killer and history. */
                if (moves[i].type != MOVE_DROP ||
                    check_winner(&child) != s->turn) {
                    store_killer(ply, &moves[i]);
                    history_update(&moves[i], depth);
                }
                break;
            }
        }
    } else {
        best = SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child;
            state_copy(&child, s);
            if (moves[i].type == MOVE_DROP) apply_drop(&child, moves[i].col);
            else apply_remove(&child, moves[i].col, moves[i].row);
            switch_turn(&child);

            int val;
            Move child_best;
            if (i == 0) {
                val = pvs_search(&child, depth - 1, ply + 1,
                                 alpha, beta, &child_best, m);
            } else {
                val = pvs_search(&child, depth - 1, ply + 1,
                                 beta - 1, beta, &child_best, m);
                if (val < beta && val > alpha) {
                    val = pvs_search(&child, depth - 1, ply + 1,
                                     alpha, beta, &child_best, m);
                }
            }

            if (val < best) { best = val; best_move = moves[i]; }
            if (best < beta) beta = best;
            if (alpha >= beta) {
                store_killer(ply, &moves[i]);
                history_update(&moves[i], depth);
                break;
            }
        }
    }

    /* Store in TT. */
    int bound;
    if (maximizing) {
        if      (best <= orig_alpha) bound = BOUND_UPPER;
        else if (best >= orig_beta)  bound = BOUND_LOWER;
        else                         bound = BOUND_EXACT;
    } else {
        if      (best >= orig_beta)  bound = BOUND_LOWER;
        else if (best <= orig_alpha) bound = BOUND_UPPER;
        else                         bound = BOUND_EXACT;
    }
    tt5_store(hash, depth, best, bound, &best_move);

    *best_out = best_move;
    return best;
}

/* ---- Public entry point ---- */

Move ai_choose_move_mode5(const GameState *state, int depth, Metrics *m) {
    Move moves[MAX_MOVES];
    int count = generate_moves(state, moves);

    if (count == 1) {
        m->depth_completed = depth;
        return moves[0];
    }

    /* Clear killers and history for a fresh search. */
    killers_clear();
    history_clear();

    Move best_move = moves[0];

    /* Aspiration window parameters. */
    int prev_score = 0;
    int asp_delta  = 50;  /* initial aspiration window half-width */

    /* Iterative deepening: depth 1, 2, ..., target. */
    for (int d = 1; d <= depth; d++) {
        int alpha, beta;

        if (d == 1) {
            /* First iteration: full window. */
            alpha = -SCORE_INF;
            beta  =  SCORE_INF;
        } else {
            /* Aspiration window around previous score. */
            alpha = prev_score - asp_delta;
            beta  = prev_score + asp_delta;
        }

        Move iter_best;
        int score;

        /* Search with aspiration window; widen on fail. */
        for (;;) {
            score = pvs_search(state, d, 0, alpha, beta, &iter_best, m);

            if (score <= alpha) {
                /* Fail low: widen alpha. */
                alpha = -SCORE_INF;
            } else if (score >= beta) {
                /* Fail high: widen beta. */
                beta = SCORE_INF;
            } else {
                break;  /* Score within window — done. */
            }
        }

        best_move  = iter_best;
        prev_score = score;
        m->depth_completed = d;

        /* Reset aspiration delta for next iteration. */
        asp_delta = 50;
    }

    return best_move;
}
