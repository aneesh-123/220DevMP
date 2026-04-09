/*
 * Removal-aware opponent: threats + center control + removal flexibility.
 *
 * Extends threat_seeker with two additions:
 *   1. Center-column weighting — pieces closer to the center are worth more
 *      because they participate in more potential winning lines.
 *   2. Removal flexibility bonus — having removals remaining is treated as a
 *      positive resource. Each remaining removal is worth a small positional
 *      bonus; having removals while the opponent has none is a meaningful edge.
 *
 * This is the strongest provided opponent. An evaluator that also reasons about
 * removal threats and gravity collapses should be able to beat it.
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

float removal_aware_evaluate(const GameState *state, int player) {
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

    /* Balanced threat counting — own and opponent weighted equally */
    score += 10.0f * (float)(count_windows(state, player,   3)
                           - count_windows(state, opponent, 3));
    score += 2.0f  * (float)(count_windows(state, player,   2)
                           - count_windows(state, opponent, 2));

    /* Center-column positional bonus */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            int dist = c - 3;
            if (dist < 0) dist = -dist;
            if (state->board[r][c] == player)
                score += (float)(3 - dist);
            else if (state->board[r][c] == opponent)
                score -= (float)(3 - dist) * 0.5f;
        }
    }

    /* Removal flexibility: each remaining removal is a strategic resource */
    score += 5.0f * (float)(state->removals_remaining[player]
                          - state->removals_remaining[opponent]);

    return score;
}
