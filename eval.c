#include "eval.h"

/*
 * Score a window of 4 cells from X's perspective.
 * Returns 0 if the window contains both X and O pieces (blocked).
 */
static int score_window(char w[4]) {
    int xc = 0, oc = 0, ec = 0;
    for (int i = 0; i < 4; i++) {
        if      (w[i] == PLAYER_X) xc++;
        else if (w[i] == PLAYER_O) oc++;
        else                       ec++;
    }

    /* Mixed window: both players present → no value. */
    if (xc > 0 && oc > 0) return 0;

    if (xc == 4) return SCORE_WIN;
    if (oc == 4) return -SCORE_WIN;
    if (xc == 3 && ec == 1) return  100;
    if (xc == 2 && ec == 2) return   10;
    if (oc == 3 && ec == 1) return -120;
    if (oc == 2 && ec == 2) return  -12;
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

    /* Diagonal up-right (/) */
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            for (int k = 0; k < 4; k++) w[k] = s->board[r + k][c + k];
            score += score_window(w);
        }
    }

    /* Diagonal up-left (\) */
    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 3; c < COLS; c++) {
            for (int k = 0; k < 4; k++) w[k] = s->board[r + k][c - k];
            score += score_window(w);
        }
    }

    /* Center column bonus */
    for (int r = 0; r < ROWS; r++) {
        if      (s->board[r][3] == PLAYER_X) score += 6;
        else if (s->board[r][3] == PLAYER_O) score -= 6;
    }

    return score;
}
