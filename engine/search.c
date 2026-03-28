#include <float.h>
#include "search.h"
#include "rules.h"

static float minimax(
    const GameState *state,
    int depth,
    EvaluateFn evaluate,
    int maximizing,
    float alpha,
    float beta,
    Move *best_move_out,
    int *has_move_out
) {
    Move legal_moves[MAX_LEGAL_MOVES];
    int num_moves, i;

    *has_move_out = 0;

    /* Base case: terminal or depth 0 */
    if (depth == 0 || state->is_terminal) {
        return evaluate(state, state->current_player);
    }

    num_moves = get_legal_moves(state, legal_moves);
    if (num_moves == 0) {
        return evaluate(state, state->current_player);
    }

    if (maximizing) {
        float max_eval = -FLT_MAX;
        for (i = 0; i < num_moves; i++) {
            GameState next = apply_move(state, legal_moves[i]);
            Move dummy;
            int dummy_has;
            float eval = minimax(&next, depth - 1, evaluate, 0,
                                 alpha, beta, &dummy, &dummy_has);
            if (eval > max_eval) {
                max_eval = eval;
                *best_move_out = legal_moves[i];
                *has_move_out = 1;
            }
            if (eval > alpha) alpha = eval;
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        float min_eval = FLT_MAX;
        for (i = 0; i < num_moves; i++) {
            GameState next = apply_move(state, legal_moves[i]);
            Move dummy;
            int dummy_has;
            float eval = minimax(&next, depth - 1, evaluate, 1,
                                 alpha, beta, &dummy, &dummy_has);
            if (eval < min_eval) {
                min_eval = eval;
                *best_move_out = legal_moves[i];
                *has_move_out = 1;
            }
            if (eval < beta) beta = eval;
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

SearchResult search(const GameState *state, int depth, EvaluateFn evaluate) {
    SearchResult result;
    result.depth_searched = depth;
    result.score = minimax(state, depth, evaluate, 1,
                           -FLT_MAX, FLT_MAX,
                           &result.best_move, &result.has_move);
    return result;
}
