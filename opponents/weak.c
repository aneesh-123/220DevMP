/*
 * Weak opponent: terminal bonus + slight center column preference.
 * Should be beatable by any reasonable multi-heuristic evaluator.
 */

#include "../engine/board.h"

float weak_evaluate(const GameState *state, int player) {
    float score = 0.0f;
    int r, c;

    /* Terminal state bonus */
    if (state->is_terminal) {
        if (state->winner == player)
            return 10000.0f;
        else if (state->winner != WINNER_NONE)
            return -10000.0f;
        else
            return 0.0f;
    }

    /* Slight center column preference */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (state->board[r][c] == player) {
                /* Center columns (2,3,4) worth more */
                int dist = c - 3;
                if (dist < 0) dist = -dist;
                score += (float)(3 - dist);
            } else if (state->board[r][c] != CELL_EMPTY) {
                int dist = c - 3;
                if (dist < 0) dist = -dist;
                score -= (float)(3 - dist) * 0.5f;
            }
        }
    }

    return score;
}
