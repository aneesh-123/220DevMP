/*
 * Reusable heuristic helper functions for evaluator design.
 *
 * All public functions take (const GameState *state, int player) and return
 * a float where higher is better for `player`.
 */

#include "heuristics.h"

/* -----------------------------------------------------------------------
 * Terminal state
 * --------------------------------------------------------------------- */

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

/* -----------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------- */

/*
 * Returns 1 if (row, col) is empty AND is the current top of its column
 * (i.e., a piece dropped into that column would land here).
 */
static int is_playable_cell(const GameState *state, int row, int col) {
    if (state->board[row][col] != CELL_EMPTY)
        return 0;
    if (row == ROWS - 1)
        return 1;
    return state->board[row + 1][col] != CELL_EMPTY;
}

/*
 * Count all windows of 4 consecutive cells (horizontal, vertical, both
 * diagonals) that contain exactly n_mine pieces of `player` and zero
 * pieces of the opponent (the remaining cells are empty).
 */
static int count_windows(const GameState *state, int player, int n_mine) {
    int opponent = 1 - player;
    int count = 0;
    int r, c, i;
    int mine, opp;

    /* Horizontal */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            mine = 0; opp = 0;
            for (i = 0; i < 4; i++) {
                if      (state->board[r][c + i] == player)   mine++;
                else if (state->board[r][c + i] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    /* Vertical */
    for (c = 0; c < COLS; c++) {
        for (r = 0; r <= ROWS - 4; r++) {
            mine = 0; opp = 0;
            for (i = 0; i < 4; i++) {
                if      (state->board[r + i][c] == player)   mine++;
                else if (state->board[r + i][c] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    /* Diagonal: top-left to bottom-right */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            mine = 0; opp = 0;
            for (i = 0; i < 4; i++) {
                if      (state->board[r + i][c + i] == player)   mine++;
                else if (state->board[r + i][c + i] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    /* Diagonal: top-right to bottom-left */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 3; c < COLS; c++) {
            mine = 0; opp = 0;
            for (i = 0; i < 4; i++) {
                if      (state->board[r + i][c - i] == player)   mine++;
                else if (state->board[r + i][c - i] == opponent) opp++;
            }
            if (opp == 0 && mine == n_mine) count++;
        }
    }

    return count;
}

/*
 * Count 3-in-a-row windows for `player` where the single empty cell is
 * gravity-reachable right now (the top of its column).
 * These are threats that can be completed or must be blocked this turn.
 */
static int count_actionable_threats(const GameState *state, int player) {
    int opponent = 1 - player;
    int count = 0;
    int r, c, i;
    int mine, opp, empty_r, empty_c;

    /* Horizontal */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            mine = 0; opp = 0; empty_r = -1; empty_c = -1;
            for (i = 0; i < 4; i++) {
                if      (state->board[r][c + i] == player)   mine++;
                else if (state->board[r][c + i] == opponent) opp++;
                else  { empty_r = r; empty_c = c + i; }
            }
            if (opp == 0 && mine == 3 && empty_r >= 0 &&
                is_playable_cell(state, empty_r, empty_c))
                count++;
        }
    }

    /* Vertical */
    for (c = 0; c < COLS; c++) {
        for (r = 0; r <= ROWS - 4; r++) {
            mine = 0; opp = 0; empty_r = -1; empty_c = -1;
            for (i = 0; i < 4; i++) {
                if      (state->board[r + i][c] == player)   mine++;
                else if (state->board[r + i][c] == opponent) opp++;
                else  { empty_r = r + i; empty_c = c; }
            }
            if (opp == 0 && mine == 3 && empty_r >= 0 &&
                is_playable_cell(state, empty_r, empty_c))
                count++;
        }
    }

    /* Diagonal: top-left to bottom-right */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            mine = 0; opp = 0; empty_r = -1; empty_c = -1;
            for (i = 0; i < 4; i++) {
                if      (state->board[r + i][c + i] == player)   mine++;
                else if (state->board[r + i][c + i] == opponent) opp++;
                else  { empty_r = r + i; empty_c = c + i; }
            }
            if (opp == 0 && mine == 3 && empty_r >= 0 &&
                is_playable_cell(state, empty_r, empty_c))
                count++;
        }
    }

    /* Diagonal: top-right to bottom-left */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 3; c < COLS; c++) {
            mine = 0; opp = 0; empty_r = -1; empty_c = -1;
            for (i = 0; i < 4; i++) {
                if      (state->board[r + i][c - i] == player)   mine++;
                else if (state->board[r + i][c - i] == opponent) opp++;
                else  { empty_r = r + i; empty_c = c - i; }
            }
            if (opp == 0 && mine == 3 && empty_r >= 0 &&
                is_playable_cell(state, empty_r, empty_c))
                count++;
        }
    }

    return count;
}

/* -----------------------------------------------------------------------
 * Public heuristics
 * --------------------------------------------------------------------- */

float open_window_score(const GameState *state, int player) {
    int opponent = 1 - player;
    float score = 0.0f;

    /* 3-in-a-row windows are far more valuable than 2-in-a-row */
    score += 10.0f * (float)(count_windows(state, player,   3)
                           - count_windows(state, opponent, 3));
    score +=  1.0f * (float)(count_windows(state, player,   2)
                           - count_windows(state, opponent, 2));

    return score;
}

float blocking_priority(const GameState *state, int player) {
    int opponent = 1 - player;

    /*
     * Own 3-windows count at 1x; opponent 3-windows penalized at 2x.
     * This breaks the symmetric assumption in open_window_score and
     * matches the blocking aggression of opponents like Defensive (which
     * penalizes opponent threats at 3x its own build weight).
     * No gravity filter — raw windows are used so developing threats
     * that aren't yet actionable are still disrupted early.
     */
    float own_3 = (float)count_windows(state, player,   3);
    float opp_3 = (float)count_windows(state, opponent, 3);

    return own_3 - 2.0f * opp_3;
}

float threat_count(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(count_actionable_threats(state, player)
                 - count_actionable_threats(state, opponent));
}

float double_threat_bonus(const GameState *state, int player) {
    int opponent   = 1 - player;
    int own_threats = count_actionable_threats(state, player);
    int opp_threats = count_actionable_threats(state, opponent);
    float score = 0.0f;

    /* 2+ simultaneous threats create a fork that cannot be fully blocked */
    if (own_threats >= 2) score += 500.0f;
    if (opp_threats >= 2) score -= 500.0f;

    return score;
}

float center_control(const GameState *state, int player) {
    int opponent = 1 - player;
    float score = 0.0f;
    int r, c, dist, weight;

    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            dist   = c - 3;
            if (dist < 0) dist = -dist;
            weight = 3 - dist; /* 3 at center col 3, down to 0 at cols 0/6 */

            if      (state->board[r][c] == player)   score += (float)weight;
            else if (state->board[r][c] == opponent) score -= (float)weight;
        }
    }

    return score;
}

float height_penalty(const GameState *state, int player) {
    int opponent = 1 - player;
    int own_high = 0, opp_high = 0;
    int r, c;

    /* Rows 0 and 1 are the top two rows — pieces here have fewer lines
       through them and are exposed as easy removal targets. */
    for (r = 0; r <= 1; r++) {
        for (c = 0; c < COLS; c++) {
            if      (state->board[r][c] == player)   own_high++;
            else if (state->board[r][c] == opponent) opp_high++;
        }
    }

    /* Positive when opponent is high (neutral-to-good for us),
       negative when we are high (bad for us). */
    return (float)(opp_high - own_high);
}

float removal_flexibility(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(state->removals_remaining[player]
                 - state->removals_remaining[opponent]);
}

float removal_threat_exposure(const GameState *state, int player) {
    int opponent = 1 - player;
    float exposure = 0.0f;
    int r, c, off, i;
    int mine, opp, start_r, start_c;

    /* If the opponent cannot remove anything, there is no exposure. */
    if (state->removals_remaining[opponent] == 0)
        return 0.0f;

    /*
     * For each of player's pieces, count how many open 3-windows it
     * anchors (windows with 3 player pieces and 0 opponent pieces that
     * include this cell). Pieces in more windows are more critical and
     * therefore more exposed to a damaging removal.
     */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (state->board[r][c] != player) continue;

            /* Horizontal windows that include column c on row r */
            {
                int s = c - 3; if (s < 0)        s = 0;
                int e = c;     if (e > COLS - 4)  e = COLS - 4;
                for (i = s; i <= e; i++) {
                    int j;
                    mine = 0; opp = 0;
                    for (j = 0; j < 4; j++) {
                        if      (state->board[r][i + j] == player)   mine++;
                        else if (state->board[r][i + j] == opponent) opp++;
                    }
                    if (opp == 0 && mine == 3) exposure += 1.0f;
                }
            }

            /* Vertical windows that include row r on column c */
            {
                int s = r - 3; if (s < 0)        s = 0;
                int e = r;     if (e > ROWS - 4)  e = ROWS - 4;
                for (i = s; i <= e; i++) {
                    int j;
                    mine = 0; opp = 0;
                    for (j = 0; j < 4; j++) {
                        if      (state->board[i + j][c] == player)   mine++;
                        else if (state->board[i + j][c] == opponent) opp++;
                    }
                    if (opp == 0 && mine == 3) exposure += 1.0f;
                }
            }

            /* Diagonal (TL->BR) windows that include (r, c) */
            for (off = 0; off <= 3; off++) {
                int j;
                start_r = r - off; start_c = c - off;
                if (start_r < 0 || start_c < 0 ||
                    start_r > ROWS - 4 || start_c > COLS - 4) continue;
                mine = 0; opp = 0;
                for (j = 0; j < 4; j++) {
                    if      (state->board[start_r + j][start_c + j] == player)   mine++;
                    else if (state->board[start_r + j][start_c + j] == opponent) opp++;
                }
                if (opp == 0 && mine == 3) exposure += 1.0f;
            }

            /* Diagonal (TR->BL) windows that include (r, c) */
            for (off = 0; off <= 3; off++) {
                int j;
                start_r = r - off; start_c = c + off;
                if (start_r < 0 || start_c >= COLS ||
                    start_r > ROWS - 4 || start_c < 3) continue;
                mine = 0; opp = 0;
                for (j = 0; j < 4; j++) {
                    if      (state->board[start_r + j][start_c - j] == player)   mine++;
                    else if (state->board[start_r + j][start_c - j] == opponent) opp++;
                }
                if (opp == 0 && mine == 3) exposure += 1.0f;
            }
        }
    }

    /* Negative: exposure is bad for the player */
    return -exposure;
}

float gravity_chain_potential(const GameState *state, int player) {
    int opponent   = 1 - player;
    int own_chains = 0, opp_chains = 0;
    int r, c;

    /*
     * Count columns where player's piece sits immediately above an
     * opponent piece. If the opponent piece is removed (by player),
     * gravity drops the player's piece one row down, which can extend
     * horizontal lines or open new diagonal threats.
     */
    for (c = 0; c < COLS; c++) {
        for (r = 0; r < ROWS - 1; r++) {
            if (state->board[r][c] == player   && state->board[r + 1][c] == opponent)
                own_chains++;
            else if (state->board[r][c] == opponent && state->board[r + 1][c] == player)
                opp_chains++;
        }
    }

    return (float)(own_chains - opp_chains);
}
