#include "ai_mode3.h"
#include "eval.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Mode 3: Alpha-Beta + Move Ordering + Transposition Table.
 *
 * Builds on Mode 2 (alpha-beta with move ordering) and adds a
 * hash table that caches evaluated positions. When the same board
 * state is reached through a different sequence of moves, the
 * cached result is reused, avoiding redundant subtree searches.
 *
 * Hashing: Zobrist hashing — each (row, col, piece) combination
 * has a random 64-bit key. The board hash is the XOR of all keys
 * for occupied cells, plus keys for the current player and
 * remaining removals.
 *
 * TT entry stores: hash key, depth, score, bound type (EXACT /
 * LOWER / UPPER). Uses always-replace policy for simplicity.
 */

/* ---- Zobrist hashing ---- */

/* zobrist_pieces[row][col][0=X, 1=O] */
static uint64_t zob_pieces[ROWS][COLS][2];
static uint64_t zob_turn;                      /* XOR when O to move */
static uint64_t zob_rem[2][MAX_REMOVALS + 1];  /* [0=X,1=O][count] */

/* Simple xorshift64* PRNG for generating Zobrist keys. */
static uint64_t zob_rng = 0x123456789ABCDEF0ULL;

static uint64_t zob_next(void) {
    zob_rng ^= zob_rng >> 12;
    zob_rng ^= zob_rng << 25;
    zob_rng ^= zob_rng >> 27;
    return zob_rng * 0x2545F4914F6CDD1DULL;
}

void mode3_init(void) {
    zob_rng = 0x123456789ABCDEF0ULL;  /* deterministic seed */
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            for (int p = 0; p < 2; p++)
                zob_pieces[r][c][p] = zob_next();
    zob_turn = zob_next();
    for (int p = 0; p < 2; p++)
        for (int k = 0; k <= MAX_REMOVALS; k++)
            zob_rem[p][k] = zob_next();
}

static uint64_t compute_hash(const GameState *s) {
    uint64_t h = 0;
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            if (s->board[r][c] == PLAYER_X) h ^= zob_pieces[r][c][0];
            else if (s->board[r][c] == PLAYER_O) h ^= zob_pieces[r][c][1];
        }
    if (s->turn == PLAYER_O) h ^= zob_turn;
    h ^= zob_rem[0][s->rem_x];
    h ^= zob_rem[1][s->rem_o];
    return h;
}

/* ---- Transposition table ---- */

#define TT_SIZE  (1 << 20)   /* ~1 million entries */
#define TT_MASK  (TT_SIZE - 1)

#define BOUND_EXACT 0
#define BOUND_LOWER 1   /* true value >= stored (alpha cutoff) */
#define BOUND_UPPER 2   /* true value <= stored (beta cutoff)  */

typedef struct {
    uint64_t key;
    int      depth;
    int      score;
    int      bound;
    int      valid;
} TTEntry;

static TTEntry tt[TT_SIZE];

void mode3_tt_clear(void) {
    memset(tt, 0, sizeof(tt));
}

static void tt_store(uint64_t key, int depth, int score, int bound) {
    int idx = (int)(key & TT_MASK);
    TTEntry *e = &tt[idx];
    e->key   = key;
    e->depth = depth;
    e->score = score;
    e->bound = bound;
    e->valid = 1;
}

/* Returns 1 if a usable entry was found, fills *out_score. */
static int tt_probe(uint64_t key, int depth, int alpha, int beta,
                    int *out_score, Metrics *m) {
    m->tt_probes++;
    int idx = (int)(key & TT_MASK);
    TTEntry *e = &tt[idx];
    if (!e->valid || e->key != key || e->depth < depth)
        return 0;
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

/* ---- Move ordering (same as Mode 2) ---- */

static int move_priority(const GameState *s, const Move *mv) {
    GameState copy;
    state_copy(&copy, s);
    if (mv->type == MOVE_DROP) apply_drop(&copy, mv->col);
    else                       apply_remove(&copy, mv->col, mv->row);
    if (check_winner(&copy) == s->turn) return 0;
    if (mv->type == MOVE_DROP) return 2 + abs(mv->col - 3);
    return 10;
}

static void order_moves(const GameState *s, Move *moves, int count) {
    int pri[MAX_MOVES];
    for (int i = 0; i < count; i++)
        pri[i] = move_priority(s, &moves[i]);
    for (int i = 1; i < count; i++) {
        Move mt = moves[i]; int pt = pri[i]; int j = i - 1;
        while (j >= 0 && pri[j] > pt) {
            moves[j+1] = moves[j]; pri[j+1] = pri[j]; j--;
        }
        moves[j+1] = mt; pri[j+1] = pt;
    }
}

/* ---- Alpha-beta with ordering + TT ---- */

static int alphabeta_tt_search(const GameState *s, int depth,
                               int alpha, int beta, Metrics *m) {
    m->nodes_visited++;

    /* Terminal checks. */
    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_board_full(s))   return 0;
    if (depth == 0)         return evaluate(s);

    /* TT lookup. */
    uint64_t hash = compute_hash(s);
    int cached;
    if (tt_probe(hash, depth, alpha, beta, &cached, m))
        return cached;

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    order_moves(s, moves, count);

    int maximizing  = (s->turn == PLAYER_X);
    int orig_alpha  = alpha;
    int orig_beta   = beta;
    int best;

    if (maximizing) {
        best = -SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child;
            state_copy(&child, s);
            if (moves[i].type == MOVE_DROP) apply_drop(&child, moves[i].col);
            else apply_remove(&child, moves[i].col, moves[i].row);
            switch_turn(&child);
            int val = alphabeta_tt_search(&child, depth - 1, alpha, beta, m);
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (alpha >= beta) break;
        }
    } else {
        best = SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child;
            state_copy(&child, s);
            if (moves[i].type == MOVE_DROP) apply_drop(&child, moves[i].col);
            else apply_remove(&child, moves[i].col, moves[i].row);
            switch_turn(&child);
            int val = alphabeta_tt_search(&child, depth - 1, alpha, beta, m);
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (alpha >= beta) break;
        }
    }

    /* Store result in TT. */
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
    tt_store(hash, depth, best, bound);

    return best;
}

/* ---- Public entry point ---- */

Move ai_choose_move_mode3(const GameState *state, int depth, Metrics *m) {
    Move moves[MAX_MOVES];
    int count = generate_moves(state, moves);

    if (count == 1) {
        m->depth_completed = depth;
        return moves[0];
    }

    order_moves(state, moves, count);

    int maximizing = (state->turn == PLAYER_X);
    int best_score = maximizing ? -SCORE_INF : SCORE_INF;
    int best_idx   = 0;
    int alpha = -SCORE_INF;
    int beta  =  SCORE_INF;

    for (int i = 0; i < count; i++) {
        GameState child;
        state_copy(&child, state);
        if (moves[i].type == MOVE_DROP) apply_drop(&child, moves[i].col);
        else apply_remove(&child, moves[i].col, moves[i].row);

        if (check_winner(&child) == state->turn) {
            m->depth_completed = depth;
            return moves[i];
        }
        switch_turn(&child);

        int val = alphabeta_tt_search(&child, depth - 1, alpha, beta, m);

        if (maximizing) {
            if (val > best_score) { best_score = val; best_idx = i; }
            if (best_score > alpha) alpha = best_score;
        } else {
            if (val < best_score) { best_score = val; best_idx = i; }
            if (best_score < beta) beta = best_score;
        }
    }

    m->depth_completed = depth;
    return moves[best_idx];
}
