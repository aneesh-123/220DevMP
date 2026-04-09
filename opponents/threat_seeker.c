/*
 * Threat-seeker opponent: terminal bonus + open window counting.
 *
 * Scans all windows of 4 cells (horizontal, vertical, diagonal) and scores:
 *   - 3-in-a-row with 1 empty cell (immediate winning threat): weight 10
 *   - 2-in-a-row with 2 empty cells (developing threat):       weight 1
 *
 * Counts own threats positively and opponent threats negatively.
 * Has no concept of blocking — it just maximizes its own threat score and
 * implicitly avoids opponent threats via negation. Useful benchmark for
 * evaluators that add explicit defensive weighting.
 */

#include "../engine/board.h"

/*
 * Count windows of 4 cells containing exactly n_mine pieces of `player`
 * and zero pieces of the opponent (the rest are empty).
 */
static int count_windows(const GameState *state, int player, int n_mine) {
    int opponent = 1 - player;
    int count = 0;
    int r, c, i;

    /* Horizontal */
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

    /* Vertical */
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

    /* Diagonal: top-left to bottom-right */
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

    /* Diagonal: top-right to bottom-left */
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

float threat_seeker_evaluate(const GameState *state, int player) {
    int opponent = 1 - player;

    if (state->is_terminal) {
        if (state->winner == player)
            return 10000.0f;
        else if (state->winner != WINNER_NONE)
            return -10000.0f;
        else
            return 0.0f;
    }

    float score = 0.0f;

    /* 3-in-a-row threats (highest value) */
    score += 10.0f * (float)(count_windows(state, player,   3)
                           - count_windows(state, opponent, 3));

    /* 2-in-a-row developing threats */
    score += 1.0f  * (float)(count_windows(state, player,   2)
                           - count_windows(state, opponent, 2));

    return score;
}

