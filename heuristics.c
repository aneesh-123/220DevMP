/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * ADD YOUR OWN HEURISTIC FUNCTIONS HERE.
 * Each function should:
 *   - Take (const GameState *state, int player)
 *   - Return a float score
 *   - Be declared in heuristics.h
 */

#include "heuristics.h"

/* -------------------------------------------------------------------------
 * terminal_state_bonus
 * Returns a large bonus for winning and a large penalty for losing.
 * Always include this in every evaluator — without it the bot won't
 * recognise wins or losses at search leaves.
 * ------------------------------------------------------------------------- */
float terminal_state_bonus(const GameState *state, int player) {
    if (!state->is_terminal)
        return 0.0f;

    if (state->winner == player)
        return 10000.0f;
    else if (state->winner != WINNER_NONE)
        return -10000.0f;
    else
        return 0.0f; /* draw */
}

/* -------------------------------------------------------------------------
 * center_control
 * Scores positional value based on which columns pieces occupy.
 * Center columns (especially col 3) connect in more directions and are
 * harder for the opponent to cut off.
 *
 * Column weights (cols 0-6): 0, 1, 2, 3, 2, 1, 0
 * ------------------------------------------------------------------------- */
float center_control(const GameState *state, int player) {
    static const float col_weight[COLS] = {
        0.0f, 1.0f, 2.0f, 3.0f, 2.0f, 1.0f, 0.0f
    };
    int opponent = 1 - player;
    float score = 0.0f;
    int row, col;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int cell = state->board[row][col];
            if (cell == player)
                score += col_weight[col];
            else if (cell == opponent)
                score -= col_weight[col];
        }
    }

    return score;
}

/* -------------------------------------------------------------------------
 * count_threats
 * Scans every window of 4 consecutive cells (horizontal, vertical,
 * and both diagonals) and scores based on partial connections:
 *
 *   Friendly 3-in-a-row with a playable empty end: +50
 *   Enemy    3-in-a-row with a playable empty end: -80  (defense weighted heavier)
 *   Friendly 2-in-a-row in a fully open window:    +5
 *   Enemy    2-in-a-row in a fully open window:    -3
 *
 * A "playable" empty cell is one that a piece can land in on the next
 * drop (either the bottom row, or the cell below it is already occupied).
 * ------------------------------------------------------------------------- */

static int is_playable(const GameState *state, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS)
        return 0;
    if (state->board[row][col] != CELL_EMPTY)
        return 0;
    return (row == ROWS - 1) || (state->board[row + 1][col] != CELL_EMPTY);
}

static float score_window(const GameState *state, int player,
                           int r, int c, int dr, int dc) {
    int opponent = 1 - player;
    int friendly = 0, enemy = 0, playable_empty = 0;
    int i;

    for (i = 0; i < 4; i++) {
        int cell = state->board[r + dr * i][c + dc * i];
        if (cell == player)
            friendly++;
        else if (cell == opponent)
            enemy++;
        else if (is_playable(state, r + dr * i, c + dc * i))
            playable_empty++;
    }

    /* Mixed window — neither player can complete it */
    if (friendly > 0 && enemy > 0)
        return 0.0f;

    if (friendly == 3 && playable_empty >= 1)
        return 50.0f;
    if (friendly == 2 && enemy == 0)
        return 5.0f;

    if (enemy == 3 && playable_empty >= 1)
        return -80.0f;
    if (enemy == 2 && friendly == 0)
        return -3.0f;

    return 0.0f;
}

float count_threats(const GameState *state, int player) {
    float score = 0.0f;
    int r, c;

    /* Horizontal */
    for (r = 0; r < ROWS; r++)
        for (c = 0; c + 3 < COLS; c++)
            score += score_window(state, player, r, c, 0, 1);

    /* Vertical */
    for (r = 0; r + 3 < ROWS; r++)
        for (c = 0; c < COLS; c++)
            score += score_window(state, player, r, c, 1, 0);

    /* Diagonal down-right */
    for (r = 0; r + 3 < ROWS; r++)
        for (c = 0; c + 3 < COLS; c++)
            score += score_window(state, player, r, c, 1, 1);

    /* Diagonal down-left */
    for (r = 0; r + 3 < ROWS; r++)
        for (c = 3; c < COLS; c++)
            score += score_window(state, player, r, c, 1, -1);

    return score;
}

/* -------------------------------------------------------------------------
 * removal_advantage
 * Returns a score based on the asymmetry in removal moves remaining.
 * Having your removal available is valuable: it can neutralise the
 * opponent's best piece or save a losing position.
 *
 *  +1.0  if only you have removal left
 *   0.0  if equal (both have it or neither does)
 *  -1.0  if only the opponent has removal left
 * ------------------------------------------------------------------------- */
float removal_advantage(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(state->removals_remaining[player]
                 - state->removals_remaining[opponent]);
}
