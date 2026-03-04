#include "ai.h"
#include <string.h>
#include <stdlib.h>

/* ---- Zobrist tables ---- */
uint64_t zobrist_pieces[ROWS][COLS][2];
uint64_t zobrist_player;
uint64_t zobrist_removals[2][MAX_REMOVALS + 1];

/* Simple 64-bit PRNG (xorshift64*) for Zobrist key generation. */
static uint64_t rng_state = 0x123456789ABCDEF0ULL;

static uint64_t rng_next(void) {
    rng_state ^= rng_state >> 12;
    rng_state ^= rng_state << 25;
    rng_state ^= rng_state >> 27;
    return rng_state * 0x2545F4914F6CDD1DULL;
}

void ai_init_zobrist(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            for (int p = 0; p < 2; p++)
                zobrist_pieces[r][c][p] = rng_next();
    zobrist_player = rng_next();
    for (int p = 0; p < 2; p++)
        for (int k = 0; k <= MAX_REMOVALS; k++)
            zobrist_removals[p][k] = rng_next();
}

uint64_t zobrist_hash(const GameState *s) {
    uint64_t h = 0;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (s->board[r][c] == PLAYER_X)
                h ^= zobrist_pieces[r][c][0];
            else if (s->board[r][c] == PLAYER_O)
                h ^= zobrist_pieces[r][c][1];
        }
    }
    if (s->current_player == PLAYER_O)
        h ^= zobrist_player;
    h ^= zobrist_removals[0][s->removals_left_X];
    h ^= zobrist_removals[1][s->removals_left_O];
    return h;
}

/* ---- Transposition table ---- */
static TTEntry tt_table[TT_SIZE];

void tt_clear(void) {
    memset(tt_table, 0, sizeof(tt_table));
}

static void tt_store(uint64_t key, int depth, int score, int bound) {
    int idx = (int)(key & TT_MASK);
    TTEntry *e = &tt_table[idx];
    /* Always-replace policy (simplest). */
    e->key   = key;
    e->depth = depth;
    e->score = score;
    e->bound = bound;
    e->valid = 1;
}

/* Returns 1 if hit and usable, fills out_score. */
static int tt_probe(uint64_t key, int depth, int alpha, int beta,
                    int *out_score, AIStats *stats) {
    stats->tt_probes++;
    int idx = (int)(key & TT_MASK);
    TTEntry *e = &tt_table[idx];
    if (!e->valid || e->key != key || e->depth < depth)
        return 0;
    stats->tt_hits++;

    if (e->bound == TT_EXACT) {
        *out_score = e->score;
        return 1;
    }
    if (e->bound == TT_LOWER && e->score >= beta) {
        *out_score = e->score;
        return 1;
    }
    if (e->bound == TT_UPPER && e->score <= alpha) {
        *out_score = e->score;
        return 1;
    }
    return 0;
}

/* ---- Heuristic evaluation ---- */

/* Score a window of 4 cells from X's perspective. */
static int score_window(char w[4]) {
    int x_count = 0, o_count = 0, empty_count = 0;
    for (int i = 0; i < 4; i++) {
        if (w[i] == PLAYER_X) x_count++;
        else if (w[i] == PLAYER_O) o_count++;
        else empty_count++;
    }
    /* Mixed window (both players present) = no value. */
    if (x_count > 0 && o_count > 0) return 0;

    if (x_count == 4) return SCORE_WIN;
    if (o_count == 4) return -SCORE_WIN;
    if (x_count == 3 && empty_count == 1) return 100;
    if (x_count == 2 && empty_count == 2) return 10;
    if (o_count == 3 && empty_count == 1) return -120; /* slightly stronger threat weight */
    if (o_count == 2 && empty_count == 2) return -12;
    return 0;
}

int evaluate(const GameState *s) {
    int score = 0;
    char w[4];

    /* Horizontal windows */
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int k = 0; k < 4; k++) w[k] = s->board[r][c + k];
            score += score_window(w);
        }
    }
    /* Vertical windows */
    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r <= ROWS - 4; r++) {
            for (int k = 0; k < 4; k++) w[k] = s->board[r + k][c];
            score += score_window(w);
        }
    }
    /* Diagonal up-right */
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int k = 0; k < 4; k++) w[k] = s->board[r + k][c + k];
            score += score_window(w);
        }
    }
    /* Diagonal up-left */
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 3; c < COLS; c++) {
            for (int k = 0; k < 4; k++) w[k] = s->board[r + k][c - k];
            score += score_window(w);
        }
    }

    /* Center column bonus */
    for (int r = 0; r < ROWS; r++) {
        if (s->board[r][3] == PLAYER_X) score += 6;
        else if (s->board[r][3] == PLAYER_O) score -= 6;
    }

    return score;
}

/* ---- Move ordering (for modes 1 and 2) ---- */

/* Priority: lower = searched first.
   0 = immediate win, 1 = block opponent win,
   2 = center drop, 3 = other drop, 4 = removal. */
static int move_priority(const GameState *s, const Move *m) {
    GameState copy = *s;

    /* Try the move and see if it wins immediately. */
    if (m->type == MOVE_DROP) {
        apply_drop(&copy, m->col);
    } else {
        apply_remove(&copy, m->col, m->row);
    }
    if (check_winner(&copy) == s->current_player) {
        return 0; /* Immediate win */
    }

    /* Check if this blocks an opponent win:
       switch to opponent, see if they had a winning drop. */
    if (m->type == MOVE_DROP) {
        /* Quick check: does opponent have a winning move in original state?
           And does our move block it? Simplified: just prioritize center. */
        int center_dist = abs(m->col - 3);
        return 2 + center_dist; /* 2..5 for drops */
    }

    return 10; /* Removals get lowest priority */
}

/* Simple insertion sort for move ordering (small arrays). */
static void order_moves(const GameState *s, Move *moves, int count) {
    int priorities[MAX_MOVES];
    for (int i = 0; i < count; i++)
        priorities[i] = move_priority(s, &moves[i]);

    for (int i = 1; i < count; i++) {
        Move m_tmp = moves[i];
        int p_tmp = priorities[i];
        int j = i - 1;
        while (j >= 0 && priorities[j] > p_tmp) {
            moves[j + 1] = moves[j];
            priorities[j + 1] = priorities[j];
            j--;
        }
        moves[j + 1] = m_tmp;
        priorities[j + 1] = p_tmp;
    }
}

/* ---- Search functions ---- */

/* Mode 0: Baseline minimax (no pruning, no caching).
   Returns score from X's perspective. */
static int minimax(const GameState *s, int depth, AIStats *stats) {
    stats->nodes_expanded++;

    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth; /* prefer faster wins */
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_draw(s)) return 0;
    if (depth == 0) return evaluate(s);

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    int maximizing = (s->current_player == PLAYER_X);
    int best = maximizing ? -SCORE_INF : SCORE_INF;

    for (int i = 0; i < count; i++) {
        GameState child = *s;
        if (moves[i].type == MOVE_DROP)
            apply_drop(&child, moves[i].col);
        else
            apply_remove(&child, moves[i].col, moves[i].row);
        switch_player(&child);

        int val = minimax(&child, depth - 1, stats);
        if (maximizing) {
            if (val > best) best = val;
        } else {
            if (val < best) best = val;
        }
    }
    return best;
}

/* Mode 1: Alpha-beta with move ordering.
   Returns score from X's perspective. */
static int alphabeta(const GameState *s, int depth, int alpha, int beta,
                     AIStats *stats) {
    stats->nodes_expanded++;

    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_draw(s)) return 0;
    if (depth == 0) return evaluate(s);

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    order_moves(s, moves, count);

    int maximizing = (s->current_player == PLAYER_X);

    if (maximizing) {
        int best = -SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child = *s;
            if (moves[i].type == MOVE_DROP)
                apply_drop(&child, moves[i].col);
            else
                apply_remove(&child, moves[i].col, moves[i].row);
            switch_player(&child);

            int val = alphabeta(&child, depth - 1, alpha, beta, stats);
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (alpha >= beta) break; /* Beta cutoff */
        }
        return best;
    } else {
        int best = SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child = *s;
            if (moves[i].type == MOVE_DROP)
                apply_drop(&child, moves[i].col);
            else
                apply_remove(&child, moves[i].col, moves[i].row);
            switch_player(&child);

            int val = alphabeta(&child, depth - 1, alpha, beta, stats);
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (alpha >= beta) break; /* Alpha cutoff */
        }
        return best;
    }
}

/* Mode 2: Alpha-beta + move ordering + transposition table.
   Returns score from X's perspective. */
static int alphabeta_tt(const GameState *s, int depth, int alpha, int beta,
                        AIStats *stats) {
    stats->nodes_expanded++;

    /* Check terminal states first (before TT, since these are exact). */
    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_draw(s)) return 0;
    if (depth == 0) return evaluate(s);

    /* Transposition table lookup */
    uint64_t hash = zobrist_hash(s);
    int tt_score;
    if (tt_probe(hash, depth, alpha, beta, &tt_score, stats)) {
        return tt_score;
    }

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    order_moves(s, moves, count);

    int maximizing = (s->current_player == PLAYER_X);
    int orig_alpha = alpha;
    int orig_beta  = beta;
    int best;

    if (maximizing) {
        best = -SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child = *s;
            if (moves[i].type == MOVE_DROP)
                apply_drop(&child, moves[i].col);
            else
                apply_remove(&child, moves[i].col, moves[i].row);
            switch_player(&child);

            int val = alphabeta_tt(&child, depth - 1, alpha, beta, stats);
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (alpha >= beta) break;
        }
    } else {
        best = SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child = *s;
            if (moves[i].type == MOVE_DROP)
                apply_drop(&child, moves[i].col);
            else
                apply_remove(&child, moves[i].col, moves[i].row);
            switch_player(&child);

            int val = alphabeta_tt(&child, depth - 1, alpha, beta, stats);
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (alpha >= beta) break;
        }
    }

    /* Store in transposition table */
    int bound;
    if (maximizing) {
        if (best <= orig_alpha)    bound = TT_UPPER;
        else if (best >= orig_beta) bound = TT_LOWER;
        else                        bound = TT_EXACT;
    } else {
        if (best >= orig_beta)     bound = TT_LOWER;
        else if (best <= orig_alpha) bound = TT_UPPER;
        else                         bound = TT_EXACT;
    }
    tt_store(hash, depth, best, bound);

    return best;
}

/* ---- Public AI entry point ---- */
Move ai_choose_move(const GameState *s, int mode, int depth, AIStats *stats) {
    stats->nodes_expanded = 0;
    stats->tt_probes = 0;
    stats->tt_hits = 0;

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);

    /* If only one legal move, return it immediately. */
    if (count == 1) return moves[0];

    /* For modes 1 and 2, order moves at the root as well. */
    if (mode >= 1) {
        order_moves(s, moves, count);
    }

    int maximizing = (s->current_player == PLAYER_X);
    int best_score = maximizing ? -SCORE_INF : SCORE_INF;
    int best_idx = 0;

    for (int i = 0; i < count; i++) {
        GameState child = *s;
        if (moves[i].type == MOVE_DROP)
            apply_drop(&child, moves[i].col);
        else
            apply_remove(&child, moves[i].col, moves[i].row);

        /* Check for immediate win before recursing. */
        char w = check_winner(&child);
        if (w == s->current_player) {
            return moves[i]; /* Take winning move immediately. */
        }

        switch_player(&child);

        int val;
        if (mode == 0)
            val = minimax(&child, depth - 1, stats);
        else if (mode == 1)
            val = alphabeta(&child, depth - 1, -SCORE_INF, SCORE_INF, stats);
        else
            val = alphabeta_tt(&child, depth - 1, -SCORE_INF, SCORE_INF, stats);

        if (maximizing) {
            if (val > best_score) { best_score = val; best_idx = i; }
        } else {
            if (val < best_score) { best_score = val; best_idx = i; }
        }
    }

    return moves[best_idx];
}
