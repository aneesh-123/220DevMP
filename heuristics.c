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

float piece_advantage(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(gamestate_count_pieces(state, player)
                 - gamestate_count_pieces(state, opponent));
}

float center_control(const GameState *state, int player) {
    int opponent = 1 - player;
    float score = 0.0f;
    /* Weight columns by distance from center (col 3): center=3, adj=2, outer=1, corner=0 */
    static const float col_weights[COLS] = {0.0f, 1.0f, 2.0f, 3.0f, 2.0f, 1.0f, 0.0f};

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int cell = gamestate_get_cell(state, row, col);
            if (cell == player)
                score += col_weights[col];
            else if (cell == opponent)
                score -= col_weights[col];
        }
    }
    return score;
}

/* Score a 4-cell window: +1 per open 3-in-a-row threat for player, -1 per opponent threat. */
static float score_window(int a, int b, int c, int d, int player) {
    int opponent = 1 - player;
    int cells[4] = {a, b, c, d};
    int p_count = 0, o_count = 0, e_count = 0;

    for (int i = 0; i < 4; i++) {
        if (cells[i] == player)        p_count++;
        else if (cells[i] == opponent) o_count++;
        else                           e_count++;
    }

    if (p_count == 3 && e_count == 1) return  1.0f;
    if (o_count == 3 && e_count == 1) return -1.0f;
    return 0.0f;
}

float threat_detection(const GameState *state, int player) {
    float score = 0.0f;
    int r, c;

    /* Horizontal */
    for (r = 0; r < ROWS; r++)
        for (c = 0; c <= COLS - 4; c++)
            score += score_window(
                gamestate_get_cell(state, r, c),
                gamestate_get_cell(state, r, c+1),
                gamestate_get_cell(state, r, c+2),
                gamestate_get_cell(state, r, c+3), player);

    /* Vertical */
    for (r = 0; r <= ROWS - 4; r++)
        for (c = 0; c < COLS; c++)
            score += score_window(
                gamestate_get_cell(state, r,   c),
                gamestate_get_cell(state, r+1, c),
                gamestate_get_cell(state, r+2, c),
                gamestate_get_cell(state, r+3, c), player);

    /* Diagonal down-right */
    for (r = 0; r <= ROWS - 4; r++)
        for (c = 0; c <= COLS - 4; c++)
            score += score_window(
                gamestate_get_cell(state, r,   c),
                gamestate_get_cell(state, r+1, c+1),
                gamestate_get_cell(state, r+2, c+2),
                gamestate_get_cell(state, r+3, c+3), player);

    /* Diagonal down-left */
    for (r = 0; r <= ROWS - 4; r++)
        for (c = 3; c < COLS; c++)
            score += score_window(
                gamestate_get_cell(state, r,   c),
                gamestate_get_cell(state, r+1, c-1),
                gamestate_get_cell(state, r+2, c-2),
                gamestate_get_cell(state, r+3, c-3), player);

    return score;
}

/* Scan all windows, applying fn to each set of 4 cells. */
#define SCAN_WINDOWS(fn) \
    do { \
        int _r, _c; \
        for (_r = 0; _r < ROWS; _r++) \
            for (_c = 0; _c <= COLS - 4; _c++) \
                score += fn(gamestate_get_cell(state,_r,_c), gamestate_get_cell(state,_r,_c+1), \
                            gamestate_get_cell(state,_r,_c+2), gamestate_get_cell(state,_r,_c+3), player); \
        for (_r = 0; _r <= ROWS - 4; _r++) \
            for (_c = 0; _c < COLS; _c++) \
                score += fn(gamestate_get_cell(state,_r,_c), gamestate_get_cell(state,_r+1,_c), \
                            gamestate_get_cell(state,_r+2,_c), gamestate_get_cell(state,_r+3,_c), player); \
        for (_r = 0; _r <= ROWS - 4; _r++) \
            for (_c = 0; _c <= COLS - 4; _c++) \
                score += fn(gamestate_get_cell(state,_r,_c), gamestate_get_cell(state,_r+1,_c+1), \
                            gamestate_get_cell(state,_r+2,_c+2), gamestate_get_cell(state,_r+3,_c+3), player); \
        for (_r = 0; _r <= ROWS - 4; _r++) \
            for (_c = 3; _c < COLS; _c++) \
                score += fn(gamestate_get_cell(state,_r,_c), gamestate_get_cell(state,_r+1,_c-1), \
                            gamestate_get_cell(state,_r+2,_c-2), gamestate_get_cell(state,_r+3,_c-3), player); \
    } while(0)

static float score_window_offensive(int a, int b, int c, int d, int player) {
    int cells[4] = {a, b, c, d};
    int p_count = 0, e_count = 0, o_count = 0;
    int i;
    for (i = 0; i < 4; i++) {
        if (cells[i] == player)          p_count++;
        else if (cells[i] == CELL_EMPTY) e_count++;
        else                             o_count++;
    }
    if (o_count > 0) return 0.0f;
    if (p_count == 3 && e_count == 1) return 1.0f;
    return 0.0f;
}

static float score_window_defensive(int a, int b, int c, int d, int player) {
    int opponent = 1 - player;
    int cells[4] = {a, b, c, d};
    int o_count = 0, e_count = 0, p_count = 0;
    int i;
    for (i = 0; i < 4; i++) {
        if (cells[i] == opponent)        o_count++;
        else if (cells[i] == CELL_EMPTY) e_count++;
        else                             p_count++;
    }
    if (p_count > 0) return 0.0f;
    if (o_count == 3 && e_count == 1) return 1.0f;
    return 0.0f;
}

float offensive_threats(const GameState *state, int player) {
    float score = 0.0f;
    SCAN_WINDOWS(score_window_offensive);
    return score;
}

float defensive_blocks(const GameState *state, int player) {
    float score = 0.0f;
    SCAN_WINDOWS(score_window_defensive);
    return score;
}

float removal_flexibility(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(state->removals_remaining[player]
                 - state->removals_remaining[opponent]);
}
