#include "ai_mode1.h"
#include "eval.h"

/*
 * Mode 1: Minimax + Alpha-Beta Pruning.
 *
 * Same scoring convention as Mode 0 (from X's perspective), but
 * adds alpha-beta bounds to prune branches that cannot affect
 * the final decision.
 *
 * Alpha = best score the maximizer (X) can guarantee so far.
 * Beta  = best score the minimizer (O) can guarantee so far.
 * When alpha >= beta, remaining siblings are pruned.
 *
 * No move ordering and no caching — just pruning on top of minimax.
 */

/* Recursive alpha-beta search. Returns score from X's perspective. */
static int alphabeta(const GameState *s, int depth, int alpha, int beta,
                     Metrics *m) {
    m->nodes_visited++;

    /* Terminal checks (same as Mode 0). */
    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_board_full(s))   return 0;
    if (depth == 0)         return evaluate(s);

    /* Generate all legal moves (same order as Mode 0). */
    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

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

            int val = alphabeta(&child, depth - 1, alpha, beta, m);
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (alpha >= beta) break;  /* Beta cutoff: O already has a better option */
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

            int val = alphabeta(&child, depth - 1, alpha, beta, m);
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (alpha >= beta) break;  /* Alpha cutoff: X already has a better option */
        }
        return best;
    }
}

Move ai_choose_move_mode1(const GameState *state, int depth, Metrics *m) {
    Move moves[MAX_MOVES];
    int count = generate_moves(state, moves);

    /* If only one legal move, return it immediately. */
    if (count == 1) {
        m->depth_completed = depth;
        return moves[0];
    }

    int maximizing = (state->turn == PLAYER_X);
    int best_score = maximizing ? -SCORE_INF : SCORE_INF;
    int best_idx   = 0;

    /* Root-level alpha-beta window starts fully open. */
    int alpha = -SCORE_INF;
    int beta  =  SCORE_INF;

    for (int i = 0; i < count; i++) {
        GameState child;
        state_copy(&child, state);

        if (moves[i].type == MOVE_DROP)
            apply_drop(&child, moves[i].col);
        else
            apply_remove(&child, moves[i].col, moves[i].row);

        /* Immediate win — take it without searching deeper. */
        if (check_winner(&child) == state->turn) {
            m->depth_completed = depth;
            return moves[i];
        }

        switch_turn(&child);

        int val = alphabeta(&child, depth - 1, alpha, beta, m);

        /* Deterministic tie-breaking: first move (lowest col/row) wins ties. */
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
