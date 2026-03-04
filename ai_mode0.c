#include "ai_mode0.h"
#include "eval.h"

/*
 * Mode 0: Baseline Minimax — no alpha-beta, no caching.
 *
 * Scoring convention: scores are always from X's perspective.
 *   X winning  → +SCORE_WIN (+ depth bonus for faster wins)
 *   O winning  → -SCORE_WIN (- depth bonus)
 *   Draw       → 0
 *   Non-terminal at depth 0 → evaluate(state)
 *
 * The depth bonus ensures the AI prefers winning sooner
 * and losing later.
 */

/* Recursive minimax. Returns score from X's perspective. */
static int minimax(const GameState *s, int depth, Metrics *m) {
    m->nodes_visited++;

    /* Terminal checks. */
    char winner = check_winner(s);
    if (winner == PLAYER_X) return  SCORE_WIN + depth;
    if (winner == PLAYER_O) return -SCORE_WIN - depth;
    if (is_board_full(s))   return 0;
    if (depth == 0)         return evaluate(s);

    /* Generate all legal moves. */
    Move moves[MAX_MOVES];
    int count = generate_moves(s, moves);
    if (count == 0) return evaluate(s);

    int maximizing = (s->turn == PLAYER_X);
    int best = maximizing ? -SCORE_INF : SCORE_INF;

    for (int i = 0; i < count; i++) {
        /* Make a copy and apply the move. */
        GameState child;
        state_copy(&child, s);

        if (moves[i].type == MOVE_DROP)
            apply_drop(&child, moves[i].col);
        else
            apply_remove(&child, moves[i].col, moves[i].row);

        switch_turn(&child);

        int val = minimax(&child, depth - 1, m);

        if (maximizing) {
            if (val > best) best = val;
        } else {
            if (val < best) best = val;
        }
    }
    return best;
}

Move ai_choose_move_mode0(const GameState *state, int depth, Metrics *m) {
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

    for (int i = 0; i < count; i++) {
        GameState child;
        state_copy(&child, state);

        if (moves[i].type == MOVE_DROP)
            apply_drop(&child, moves[i].col);
        else
            apply_remove(&child, moves[i].col, moves[i].row);

        /* Check for an immediate win — take it without searching. */
        if (check_winner(&child) == state->turn) {
            m->depth_completed = depth;
            return moves[i];
        }

        switch_turn(&child);

        int val = minimax(&child, depth - 1, m);

        /* Deterministic tie-breaking: first move (lowest col/row) wins ties.
         * Because generate_moves enumerates in (col, row) order and we
         * use strict inequality, the first equally-scored move is kept. */
        if (maximizing) {
            if (val > best_score) {
                best_score = val;
                best_idx   = i;
            }
        } else {
            if (val < best_score) {
                best_score = val;
                best_idx   = i;
            }
        }
    }

    m->depth_completed = depth;
    return moves[best_idx];
}
