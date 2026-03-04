#include "ai_mode2.h"
#include "eval.h"
#include <stdlib.h>

/*
 * Mode 2: Minimax + Alpha-Beta + Move Ordering.
 *
 * Same alpha-beta pruning as Mode 1, but moves are sorted before
 * searching so that the best-looking moves are tried first. This
 * causes more cutoffs, dramatically reducing nodes expanded.
 *
 * Move ordering priority (lower = searched first):
 *   0  — move that immediately wins the game
 *   2+ — drops, biased toward the center column (col 3 = priority 2,
 *         edges = priority 5)
 *   10 — removals (tried last since they're usually weaker)
 */

/* ---- Move ordering ---- */

/* Compute a priority score for a single move (lower = try first). */
static int move_priority(const GameState *s, const Move *mv) {
    /* Try the move on a copy to check for immediate win. */
    GameState copy;
    state_copy(&copy, s);

    if (mv->type == MOVE_DROP)
        apply_drop(&copy, mv->col);
    else
        apply_remove(&copy, mv->col, mv->row);

    if (check_winner(&copy) == s->turn)
        return 0;  /* Immediate win — highest priority */

    /* Drops: prefer center columns. abs(col - 3) gives 0..3. */
    if (mv->type == MOVE_DROP)
        return 2 + abs(mv->col - 3);  /* 2..5 */

    return 10;  /* Removals last */
}

/* Sort moves by priority using insertion sort (arrays are small). */
static void order_moves(const GameState *s, Move *moves, int count) {
    int priorities[MAX_MOVES];
    for (int i = 0; i < count; i++)
        priorities[i] = move_priority(s, &moves[i]);

    for (int i = 1; i < count; i++) {
        Move mv_tmp = moves[i];
        int  p_tmp  = priorities[i];
        int j = i - 1;
        while (j >= 0 && priorities[j] > p_tmp) {
            moves[j + 1]      = moves[j];
            priorities[j + 1] = priorities[j];
            j--;
        }
        moves[j + 1]      = mv_tmp;
        priorities[j + 1] = p_tmp;
    }
}

/* ---- Alpha-beta with move ordering ---- */

static int alphabeta_ordered(const GameState *s, int depth,
                             int alpha, int beta, Metrics *m) {
    m->nodes_visited++;

    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_board_full(s))   return 0;
    if (depth == 0)         return evaluate(s);

    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    /* Sort moves so the most promising are searched first. */
    order_moves(s, moves, count);

    int maximizing = (s->turn == PLAYER_X);

    if (maximizing) {
        int best = -SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child;
            state_copy(&child, s);

            if (moves[i].type == MOVE_DROP)
                apply_drop(&child, moves[i].col);
            else
                apply_remove(&child, moves[i].col, moves[i].row);

            switch_turn(&child);

            int val = alphabeta_ordered(&child, depth - 1, alpha, beta, m);
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (alpha >= beta) break;
        }
        return best;
    } else {
        int best = SCORE_INF;
        for (int i = 0; i < count; i++) {
            GameState child;
            state_copy(&child, s);

            if (moves[i].type == MOVE_DROP)
                apply_drop(&child, moves[i].col);
            else
                apply_remove(&child, moves[i].col, moves[i].row);

            switch_turn(&child);

            int val = alphabeta_ordered(&child, depth - 1, alpha, beta, m);
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (alpha >= beta) break;
        }
        return best;
    }
}

/* ---- Public entry point ---- */

Move ai_choose_move_mode2(const GameState *state, int depth, Metrics *m) {
    Move moves[MAX_MOVES];
    int count = generate_moves(state, moves);

    if (count == 1) {
        m->depth_completed = depth;
        return moves[0];
    }

    /* Order moves at the root level too. */
    order_moves(state, moves, count);

    int maximizing = (state->turn == PLAYER_X);
    int best_score = maximizing ? -SCORE_INF : SCORE_INF;
    int best_idx   = 0;
    int alpha = -SCORE_INF;
    int beta  =  SCORE_INF;

    for (int i = 0; i < count; i++) {
        GameState child;
        state_copy(&child, state);

        if (moves[i].type == MOVE_DROP)
            apply_drop(&child, moves[i].col);
        else
            apply_remove(&child, moves[i].col, moves[i].row);

        /* Immediate win — take it. */
        if (check_winner(&child) == state->turn) {
            m->depth_completed = depth;
            return moves[i];
        }

        switch_turn(&child);

        int val = alphabeta_ordered(&child, depth - 1, alpha, beta, m);

        if (maximizing) {
            if (val > best_score) {
                best_score = val;
                best_idx   = i;
            }
            if (best_score > alpha) alpha = best_score;
        } else {
            if (val < best_score) {
                best_score = val;
                best_idx   = i;
            }
            if (best_score < beta) beta = best_score;
        }
    }

    m->depth_completed = depth;
    return moves[best_idx];
}
