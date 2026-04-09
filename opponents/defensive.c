/*
 * Defensive opponent: terminal bonus + asymmetric threat weighting.
 *
 * Uses the same window-counting approach as threat_seeker, but weights
 * opponent 3-in-a-row threats much more heavily than own threats.
 * Also adds a center-column positional bonus.
 *
 * This bot prioritizes not losing over winning — it will almost always
 * block an opponent's immediate threat but may miss winning moves if they
 * require lookahead. Good for testing whether your evaluator can consistently
 * exploit an overly cautious opponent.
 */

#include "../engine/board.h"

static int count_windows(const GameState *state, int player, int n_mine) {
    int opponent = 1 - player;
    int count = 0;
    int r, c, i;

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            int mine = 0, opp = 0;
            for (i = 0; i < 4; i++) {
                if (state->board[r][c + i] == player)        mine++;
                else if (state->board[r][c + i] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    for (c = 0; c < COLS; c++) {
        for (r = 0; r <= ROWS - 4; r++) {
            int mine = 0, opp = 0;
            for (i = 0; i < 4; i++) {
                if (state->board[r + i][c] == player)        mine++;
                else if (state->board[r + i][c] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            int mine = 0, opp = 0;
            for (i = 0; i < 4; i++) {
                if (state->board[r + i][c + i] == player)        mine++;
                else if (state->board[r + i][c + i] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 3; c < COLS; c++) {
            int mine = 0, opp = 0;
            for (i = 0; i < 4; i++) {
                if (state->board[r + i][c - i] == player)        mine++;
                else if (state->board[r + i][c - i] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    return count;
}

float defensive_evaluate(const GameState *state, int player) {
    int opponent = 1 - player;
    int r, c;

    if (state->is_terminal) {
        if (state->winner == player)
            return 10000.0f;
        else if (state->winner != WINNER_NONE)
            return -10000.0f;
        else
            return 0.0f;
    }

    float score = 0.0f;

    /* Own threats (modest weight — not the priority) */
    score += 5.0f  * (float)count_windows(state, player,   3);
    score += 1.0f  * (float)count_windows(state, player,   2);

    /* Opponent threats (heavily penalized — blocking is the priority) */
    score -= 15.0f * (float)count_windows(state, opponent, 3);
    score -= 3.0f  * (float)count_windows(state, opponent, 2);

    /* Center column positional bonus */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            int dist = c - 3;
            if (dist < 0) dist = -dist;
            if (state->board[r][c] == player)
                score += (float)(3 - dist) * 0.5f;
        }
    }

    return score;
}
