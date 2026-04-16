/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * Each function evaluates one aspect of the board position.
 * Positive scores favor 'player', negative scores favor the opponent.
 * All heuristics return 0 for terminal states (use terminal_state_bonus first).
 */

#include "heuristics.h"

/* ================================================================
 * terminal_state_bonus
 * ================================================================ */

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

/* ================================================================
 * piece_count
 * ================================================================ */

float piece_count(const GameState *state, int player) {
    if (state->is_terminal) return 0.0f;

    int opponent = 1 - player;
    return (float)(gamestate_count_pieces(state, player)
                 - gamestate_count_pieces(state, opponent));
}

/* ================================================================
 * center_control
 * ================================================================ */

float center_control(const GameState *state, int player) {
    if (state->is_terminal) return 0.0f;

    /* Weight by distance from center column (col 3). */
    static const int col_weights[COLS] = {1, 2, 3, 4, 3, 2, 1};
    int opponent = 1 - player;
    float score = 0.0f;
    int r, c;

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            int cell = state->board[r][c];
            if (cell == player)
                score += (float)col_weights[c];
            else if (cell == opponent)
                score -= (float)col_weights[c];
        }
    }

    return score;
}

/* ================================================================
 * window_score  (core positional heuristic)
 * ================================================================ */

/*
 * Score a single 4-cell window.
 *
 * p_count: number of player's pieces in this window.
 * o_count: number of opponent's pieces in this window.
 *
 * A window is "blocked" (worthless to both sides) if it contains
 * pieces from both players. Otherwise it is scored based on how
 * many pieces the dominant player has filled in.
 *
 * Scoring scale:
 *   3 + 1 empty  -> ±50  (one move from winning — high urgency)
 *   2 + 2 empty  -> ±5   (developing threat)
 *   1 + 3 empty  -> ±1   (very early presence)
 */
static float score_window(int p_count, int o_count) {
    if (p_count > 0 && o_count > 0)
        return 0.0f; /* blocked window */

    if (p_count == 4) return  500.0f;
    if (p_count == 3) return   50.0f;
    if (p_count == 2) return    5.0f;
    if (p_count == 1) return    1.0f;

    if (o_count == 4) return -500.0f;
    if (o_count == 3) return  -50.0f;
    if (o_count == 2) return   -5.0f;
    if (o_count == 1) return   -1.0f;

    return 0.0f; /* all empty */
}

float window_score(const GameState *state, int player) {
    if (state->is_terminal) return 0.0f;

    int opponent = 1 - player;
    float total = 0.0f;
    int r, c, i;
    int p_count, o_count;
    int cell;

    /* ---- Horizontal windows ---- */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r][c + i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            total += score_window(p_count, o_count);
        }
    }

    /* ---- Vertical windows ---- */
    for (c = 0; c < COLS; c++) {
        for (r = 0; r <= ROWS - 4; r++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            total += score_window(p_count, o_count);
        }
    }

    /* ---- Diagonal: top-left to bottom-right ---- */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c + i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            total += score_window(p_count, o_count);
        }
    }

    /* ---- Diagonal: top-right to bottom-left ---- */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 3; c < COLS; c++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c - i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            total += score_window(p_count, o_count);
        }
    }

    return total;
}

/* ================================================================
 * threat_count
 * ================================================================ */

/*
 * Count windows with exactly 3 of one player's pieces and 1 empty.
 * These are "live threats" — one move away from winning.
 *
 * Returns player_threats - opponent_threats.
 *
 * Significance of multiple threats:
 *   1 threat  -> opponent can still block
 *   2+ threats -> opponent can only block one; the other wins (fork)
 */
float threat_count(const GameState *state, int player) {
    if (state->is_terminal) return 0.0f;

    int opponent = 1 - player;
    int p_threats = 0, o_threats = 0;
    int r, c, i;
    int p_count, o_count, empty_count;
    int cell;

    /* Macro to tally one window into threat counters */
#define TALLY_WINDOW() \
    if (p_count == 3 && o_count == 0 && empty_count == 1) p_threats++; \
    if (o_count == 3 && p_count == 0 && empty_count == 1) o_threats++;

    /* ---- Horizontal ---- */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            p_count = 0; o_count = 0; empty_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r][c + i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
                else                       empty_count++;
            }
            TALLY_WINDOW();
        }
    }

    /* ---- Vertical ---- */
    for (c = 0; c < COLS; c++) {
        for (r = 0; r <= ROWS - 4; r++) {
            p_count = 0; o_count = 0; empty_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
                else                       empty_count++;
            }
            TALLY_WINDOW();
        }
    }

    /* ---- Diagonal: top-left to bottom-right ---- */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            p_count = 0; o_count = 0; empty_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c + i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
                else                       empty_count++;
            }
            TALLY_WINDOW();
        }
    }

    /* ---- Diagonal: top-right to bottom-left ---- */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 3; c < COLS; c++) {
            p_count = 0; o_count = 0; empty_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c - i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
                else                       empty_count++;
            }
            TALLY_WINDOW();
        }
    }

#undef TALLY_WINDOW

    return (float)(p_threats - o_threats);
}

/* ================================================================
 * removal_advantage
 * ================================================================ */

/*
 * Difference in remaining removal moves.
 *
 * Having more removals than the opponent provides tactical options:
 *   - Disrupt opponent 3-in-a-rows by removing a key piece.
 *   - Create winning lines via gravity (removing a piece causes
 *     pieces above to fall, potentially completing 4-in-a-row).
 *
 * Returns a signed float: positive = player has more removals.
 */
float removal_advantage(const GameState *state, int player) {
    if (state->is_terminal) return 0.0f;

    int opponent = 1 - player;
    return (float)(state->removals_remaining[player]
                 - state->removals_remaining[opponent]);
}
