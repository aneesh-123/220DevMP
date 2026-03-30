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

static int other_player(int player) {
    return 1 - player;
}

static int is_playable_cell(const GameState *state, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS)
        return 0;
    if (state->board[row][col] != CELL_EMPTY)
        return 0;
    return row == ROWS - 1 || state->board[row + 1][col] != CELL_EMPTY;
}

static int is_removable_support_piece(const GameState *state, int row, int col) {
    if (row < 0 || row >= ROWS - 1 || col < 0 || col >= COLS)
        return 0;
    if (state->board[row][col] == CELL_EMPTY)
        return 0;
    return state->board[row + 1][col] != CELL_EMPTY;
}

static int is_supported_piece(const GameState *state, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS)
        return 0;
    if (state->board[row][col] == CELL_EMPTY)
        return 0;
    return row == ROWS - 1 || state->board[row + 1][col] != CELL_EMPTY;
}

static void apply_removal_to_copy(const GameState *state, GameState *copy, int row, int col) {
    int src;

    *copy = *state;
    copy->board[row][col] = CELL_EMPTY;

    for (src = row - 1; src >= 0; src--) {
        if (copy->board[src][col] != CELL_EMPTY) {
            copy->board[src + 1][col] = copy->board[src][col];
            copy->board[src][col] = CELL_EMPTY;
        }
    }
}

static int has_four_for_player(const GameState *state, int player) {
    int row;
    int col;
    int i;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            int ok = 1;
            for (i = 0; i < 4; i++) {
                if (state->board[row][col + i] != player) {
                    ok = 0;
                    break;
                }
            }
            if (ok)
                return 1;
        }
    }

    for (col = 0; col < COLS; col++) {
        for (row = 0; row <= ROWS - 4; row++) {
            int ok = 1;
            for (i = 0; i < 4; i++) {
                if (state->board[row + i][col] != player) {
                    ok = 0;
                    break;
                }
            }
            if (ok)
                return 1;
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            int ok = 1;
            for (i = 0; i < 4; i++) {
                if (state->board[row + i][col + i] != player) {
                    ok = 0;
                    break;
                }
            }
            if (ok)
                return 1;
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 3; col < COLS; col++) {
            int ok = 1;
            for (i = 0; i < 4; i++) {
                if (state->board[row + i][col - i] != player) {
                    ok = 0;
                    break;
                }
            }
            if (ok)
                return 1;
        }
    }

    return 0;
}

static int count_immediate_removal_wins(const GameState *state, int target_player) {
    int row;
    int col;
    int count = 0;
    GameState copy;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            if (state->board[row][col] == CELL_EMPTY)
                continue;

            apply_removal_to_copy(state, &copy, row, col);
            if (has_four_for_player(&copy, target_player))
                count++;
        }
    }

    return count;
}

static float score_window_counts(int own_count, int opp_count, int empty_count) {
    if (own_count > 0 && opp_count > 0)
        return 0.0f;
    if (own_count == 4)
        return 1000.0f;
    if (own_count == 3 && empty_count == 1)
        return 90.0f;
    if (own_count == 2 && empty_count == 2)
        return 18.0f;
    if (own_count == 1 && empty_count == 3)
        return 3.0f;
    if (opp_count == 4)
        return -1000.0f;
    if (opp_count == 3 && empty_count == 1)
        return -110.0f;
    if (opp_count == 2 && empty_count == 2)
        return -16.0f;
    if (opp_count == 1 && empty_count == 3)
        return -2.0f;
    return 0.0f;
}

static void analyze_window(const GameState *state,
                           int player,
                           int row,
                           int col,
                           int d_row,
                           int d_col,
                           float *window_score,
                           float *playable_score,
                           int *our_threats,
                           int *opp_threats,
                           int *our_live_windows,
                           int *opp_live_windows) {
    int own_count = 0;
    int opp_count = 0;
    int empty_count = 0;
    int empty_row = -1;
    int empty_col = -1;
    int i;
    int opponent = other_player(player);

    for (i = 0; i < 4; i++) {
        int cell = state->board[row + i * d_row][col + i * d_col];
        if (cell == player) {
            own_count++;
        } else if (cell == opponent) {
            opp_count++;
        } else {
            empty_count++;
            empty_row = row + i * d_row;
            empty_col = col + i * d_col;
        }
    }

    *window_score += score_window_counts(own_count, opp_count, empty_count);

    if (opp_count == 0 && own_count > 0)
        (*our_live_windows)++;
    if (own_count == 0 && opp_count > 0)
        (*opp_live_windows)++;

    if (empty_count == 1 && is_playable_cell(state, empty_row, empty_col)) {
        if (own_count == 3 && opp_count == 0) {
            *playable_score += 140.0f;
            (*our_threats)++;
        } else if (opp_count == 3 && own_count == 0) {
            *playable_score -= 170.0f;
            (*opp_threats)++;
        } else if (own_count == 2 && opp_count == 0) {
            *playable_score += 20.0f;
        } else if (opp_count == 2 && own_count == 0) {
            *playable_score -= 24.0f;
        }
    }
}

static void scan_windows(const GameState *state,
                         int player,
                         float *window_score,
                         float *playable_score,
                         int *our_threats,
                         int *opp_threats,
                         int *our_live_windows,
                         int *opp_live_windows) {
    int row;
    int col;

    *window_score = 0.0f;
    *playable_score = 0.0f;
    *our_threats = 0;
    *opp_threats = 0;
    *our_live_windows = 0;
    *opp_live_windows = 0;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            analyze_window(state, player, row, col, 0, 1,
                           window_score, playable_score, our_threats, opp_threats,
                           our_live_windows, opp_live_windows);
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 0; col < COLS; col++) {
            analyze_window(state, player, row, col, 1, 0,
                           window_score, playable_score, our_threats, opp_threats,
                           our_live_windows, opp_live_windows);
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            analyze_window(state, player, row, col, 1, 1,
                           window_score, playable_score, our_threats, opp_threats,
                           our_live_windows, opp_live_windows);
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 3; col < COLS; col++) {
            analyze_window(state, player, row, col, 1, -1,
                           window_score, playable_score, our_threats, opp_threats,
                           our_live_windows, opp_live_windows);
        }
    }
}

static void scan_tactical_patterns(const GameState *state,
                                   int player,
                                   int *our_playable_threats,
                                   int *opp_playable_threats,
                                   int *our_fake_threats,
                                   int *opp_fake_threats,
                                   int *our_distinct_threat_cols,
                                   int *opp_distinct_threat_cols) {
    int row;
    int col;
    int i;
    int opponent = other_player(player);
    int our_cols[COLS] = { 0 };
    int opp_cols[COLS] = { 0 };

    *our_playable_threats = 0;
    *opp_playable_threats = 0;
    *our_fake_threats = 0;
    *opp_fake_threats = 0;
    *our_distinct_threat_cols = 0;
    *opp_distinct_threat_cols = 0;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            int own_count = 0;
            int opp_count = 0;
            int empty_count = 0;
            int empty_row = -1;
            int empty_col = -1;

            for (i = 0; i < 4; i++) {
                int cell = state->board[row][col + i];
                if (cell == player)
                    own_count++;
                else if (cell == opponent)
                    opp_count++;
                else {
                    empty_count++;
                    empty_row = row;
                    empty_col = col + i;
                }
            }

            if (empty_count == 1 && own_count == 3 && opp_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*our_playable_threats)++;
                    if (!our_cols[empty_col]) {
                        our_cols[empty_col] = 1;
                        (*our_distinct_threat_cols)++;
                    }
                } else {
                    (*our_fake_threats)++;
                }
            } else if (empty_count == 1 && opp_count == 3 && own_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*opp_playable_threats)++;
                    if (!opp_cols[empty_col]) {
                        opp_cols[empty_col] = 1;
                        (*opp_distinct_threat_cols)++;
                    }
                } else {
                    (*opp_fake_threats)++;
                }
            }
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 0; col < COLS; col++) {
            int own_count = 0;
            int opp_count = 0;
            int empty_count = 0;
            int empty_row = -1;
            int empty_col = -1;

            for (i = 0; i < 4; i++) {
                int cell = state->board[row + i][col];
                if (cell == player)
                    own_count++;
                else if (cell == opponent)
                    opp_count++;
                else {
                    empty_count++;
                    empty_row = row + i;
                    empty_col = col;
                }
            }

            if (empty_count == 1 && own_count == 3 && opp_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*our_playable_threats)++;
                    if (!our_cols[empty_col]) {
                        our_cols[empty_col] = 1;
                        (*our_distinct_threat_cols)++;
                    }
                } else {
                    (*our_fake_threats)++;
                }
            } else if (empty_count == 1 && opp_count == 3 && own_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*opp_playable_threats)++;
                    if (!opp_cols[empty_col]) {
                        opp_cols[empty_col] = 1;
                        (*opp_distinct_threat_cols)++;
                    }
                } else {
                    (*opp_fake_threats)++;
                }
            }
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            int own_count = 0;
            int opp_count = 0;
            int empty_count = 0;
            int empty_row = -1;
            int empty_col = -1;

            for (i = 0; i < 4; i++) {
                int cell = state->board[row + i][col + i];
                if (cell == player)
                    own_count++;
                else if (cell == opponent)
                    opp_count++;
                else {
                    empty_count++;
                    empty_row = row + i;
                    empty_col = col + i;
                }
            }

            if (empty_count == 1 && own_count == 3 && opp_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*our_playable_threats)++;
                    if (!our_cols[empty_col]) {
                        our_cols[empty_col] = 1;
                        (*our_distinct_threat_cols)++;
                    }
                } else {
                    (*our_fake_threats)++;
                }
            } else if (empty_count == 1 && opp_count == 3 && own_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*opp_playable_threats)++;
                    if (!opp_cols[empty_col]) {
                        opp_cols[empty_col] = 1;
                        (*opp_distinct_threat_cols)++;
                    }
                } else {
                    (*opp_fake_threats)++;
                }
            }
        }
    }

    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 3; col < COLS; col++) {
            int own_count = 0;
            int opp_count = 0;
            int empty_count = 0;
            int empty_row = -1;
            int empty_col = -1;

            for (i = 0; i < 4; i++) {
                int cell = state->board[row + i][col - i];
                if (cell == player)
                    own_count++;
                else if (cell == opponent)
                    opp_count++;
                else {
                    empty_count++;
                    empty_row = row + i;
                    empty_col = col - i;
                }
            }

            if (empty_count == 1 && own_count == 3 && opp_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*our_playable_threats)++;
                    if (!our_cols[empty_col]) {
                        our_cols[empty_col] = 1;
                        (*our_distinct_threat_cols)++;
                    }
                } else {
                    (*our_fake_threats)++;
                }
            } else if (empty_count == 1 && opp_count == 3 && own_count == 0) {
                if (is_playable_cell(state, empty_row, empty_col)) {
                    (*opp_playable_threats)++;
                    if (!opp_cols[empty_col]) {
                        opp_cols[empty_col] = 1;
                        (*opp_distinct_threat_cols)++;
                    }
                } else {
                    (*opp_fake_threats)++;
                }
            }
        }
    }
}

static float count_weighted_windows_through_cell(const GameState *state, int player, int row, int col) {
    float score = 0.0f;
    int start_offset;
    int step;
    int opponent = other_player(player);

    for (start_offset = -3; start_offset <= 0; start_offset++) {
        int start_col = col + start_offset;
        int own_count = 0;
        int empty_count = 0;

        if (start_col < 0 || start_col + 3 >= COLS)
            continue;

        for (step = 0; step < 4; step++) {
            int cell = state->board[row][start_col + step];
            if (cell == opponent)
                break;
            if (cell == player)
                own_count++;
            else
                empty_count++;
        }

        if (step == 4)
            score += 6.0f * (float)own_count + 1.5f * (float)(4 - empty_count);
    }

    for (start_offset = -3; start_offset <= 0; start_offset++) {
        int start_row = row + start_offset;
        int own_count = 0;
        int empty_count = 0;

        if (start_row < 0 || start_row + 3 >= ROWS)
            continue;

        for (step = 0; step < 4; step++) {
            int cell = state->board[start_row + step][col];
            if (cell == opponent)
                break;
            if (cell == player)
                own_count++;
            else
                empty_count++;
        }

        if (step == 4)
            score += 6.0f * (float)own_count + 1.5f * (float)(4 - empty_count);
    }

    for (start_offset = -3; start_offset <= 0; start_offset++) {
        int start_row = row + start_offset;
        int start_col = col + start_offset;
        int own_count = 0;
        int empty_count = 0;

        if (start_row < 0 || start_row + 3 >= ROWS || start_col < 0 || start_col + 3 >= COLS)
            continue;

        for (step = 0; step < 4; step++) {
            int cell = state->board[start_row + step][start_col + step];
            if (cell == opponent)
                break;
            if (cell == player)
                own_count++;
            else
                empty_count++;
        }

        if (step == 4)
            score += 7.0f * (float)own_count + 1.5f * (float)(4 - empty_count);
    }

    for (start_offset = -3; start_offset <= 0; start_offset++) {
        int start_row = row + start_offset;
        int start_col = col - start_offset;
        int own_count = 0;
        int empty_count = 0;

        if (start_row < 0 || start_row + 3 >= ROWS || start_col >= COLS || start_col - 3 < 0)
            continue;

        for (step = 0; step < 4; step++) {
            int cell = state->board[start_row + step][start_col - step];
            if (cell == opponent)
                break;
            if (cell == player)
                own_count++;
            else
                empty_count++;
        }

        if (step == 4)
            score += 7.0f * (float)own_count + 1.5f * (float)(4 - empty_count);
    }

    return score;
}

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

float center_control(const GameState *state, int player) {
    static const float column_weights[COLS] = { 3.0f, 4.0f, 6.0f, 7.0f, 6.0f, 4.0f, 3.0f };
    float score = 0.0f;
    int row;
    int col;
    int opponent = other_player(player);

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            if (state->board[row][col] == player)
                score += column_weights[col];
            else if (state->board[row][col] == opponent)
                score -= column_weights[col];
        }
    }

    return score;
}

float window_strength(const GameState *state, int player) {
    float window_score;
    float playable_score;
    int our_threats;
    int opp_threats;
    int our_live_windows;
    int opp_live_windows;

    scan_windows(state, player, &window_score, &playable_score, &our_threats,
                 &opp_threats, &our_live_windows, &opp_live_windows);
    return window_score;
}

float playable_threats(const GameState *state, int player) {
    float window_score;
    float playable_score;
    int our_threats;
    int opp_threats;
    int our_live_windows;
    int opp_live_windows;

    scan_windows(state, player, &window_score, &playable_score, &our_threats,
                 &opp_threats, &our_live_windows, &opp_live_windows);
    return playable_score;
}

float immediate_loss_alarm(const GameState *state, int player) {
    float window_score;
    float playable_score;
    int our_threats;
    int opp_threats;
    int our_live_windows;
    int opp_live_windows;

    scan_windows(state, player, &window_score, &playable_score, &our_threats,
                 &opp_threats, &our_live_windows, &opp_live_windows);

    if (opp_threats == 0)
        return 0.0f;

    return -250.0f * (float)opp_threats;
}

float double_threat_pressure(const GameState *state, int player) {
    float window_score;
    float playable_score;
    int our_threats;
    int opp_threats;
    int our_live_windows;
    int opp_live_windows;
    float score = 0.0f;

    scan_windows(state, player, &window_score, &playable_score, &our_threats,
                 &opp_threats, &our_live_windows, &opp_live_windows);

    if (our_threats >= 2)
        score += 180.0f + 60.0f * (float)(our_threats - 2);
    if (opp_threats >= 2)
        score -= 220.0f + 80.0f * (float)(opp_threats - 2);

    return score;
}

float line_potential(const GameState *state, int player) {
    float window_score;
    float playable_score;
    int our_threats;
    int opp_threats;
    int our_live_windows;
    int opp_live_windows;

    scan_windows(state, player, &window_score, &playable_score, &our_threats,
                 &opp_threats, &our_live_windows, &opp_live_windows);

    return 5.0f * (float)(our_live_windows - opp_live_windows);
}

float supported_connections(const GameState *state, int player) {
    float score = 0.0f;
    int row;
    int col;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int cell = state->board[row][col];
            if (cell == CELL_EMPTY || !is_supported_piece(state, row, col))
                continue;

            if (col + 1 < COLS && state->board[row][col + 1] == cell)
                score += (cell == player) ? 10.0f : -10.0f;
            if (row + 1 < ROWS && state->board[row + 1][col] == cell)
                score += (cell == player) ? 8.0f : -8.0f;
            if (row + 1 < ROWS && col + 1 < COLS && state->board[row + 1][col + 1] == cell)
                score += (cell == player) ? 11.0f : -11.0f;
            if (row + 1 < ROWS && col - 1 >= 0 && state->board[row + 1][col - 1] == cell)
                score += (cell == player) ? 11.0f : -11.0f;
        }
    }

    return score;
}

float forced_block_value(const GameState *state, int player) {
    int our_playable_threats;
    int opp_playable_threats;
    int our_fake_threats;
    int opp_fake_threats;
    int our_distinct_threat_cols;
    int opp_distinct_threat_cols;
    float score = 0.0f;

    scan_tactical_patterns(state, player,
                           &our_playable_threats, &opp_playable_threats,
                           &our_fake_threats, &opp_fake_threats,
                           &our_distinct_threat_cols, &opp_distinct_threat_cols);

    if (our_playable_threats > 0 && opp_playable_threats == 0)
        score += 65.0f + 25.0f * (float)(our_distinct_threat_cols - 1);
    if (opp_playable_threats > 0 && our_playable_threats == 0)
        score -= 75.0f + 25.0f * (float)(opp_distinct_threat_cols - 1);

    return score;
}

float unstable_threat_penalty(const GameState *state, int player) {
    int our_playable_threats;
    int opp_playable_threats;
    int our_fake_threats;
    int opp_fake_threats;
    int our_distinct_threat_cols;
    int opp_distinct_threat_cols;
    float score = 0.0f;

    scan_tactical_patterns(state, player,
                           &our_playable_threats, &opp_playable_threats,
                           &our_fake_threats, &opp_fake_threats,
                           &our_distinct_threat_cols, &opp_distinct_threat_cols);

    score -= 28.0f * (float)our_fake_threats;
    score += 28.0f * (float)opp_fake_threats;

    if (our_playable_threats > 0 && opp_playable_threats > 0)
        score -= 20.0f * (float)(opp_playable_threats - our_playable_threats + 1);

    if (our_playable_threats > 0 && our_distinct_threat_cols == 1)
        score -= 10.0f;
    if (opp_playable_threats > 0 && opp_distinct_threat_cols == 1)
        score += 10.0f;

    return score;
}

float immediate_removal_win_swing(const GameState *state, int player) {
    int opponent = other_player(player);
    int our_wins = 0;
    int opp_wins = 0;

    if (state->removals_remaining[player] > 0)
        our_wins = count_immediate_removal_wins(state, player);
    if (state->removals_remaining[opponent] > 0)
        opp_wins = count_immediate_removal_wins(state, opponent);

    return 220.0f * (float)our_wins - 260.0f * (float)opp_wins;
}

float removal_availability_value(const GameState *state, int player) {
    int opponent = other_player(player);
    float score = 0.0f;

    if (state->removals_remaining[player] > 0)
        score += 45.0f;
    if (state->removals_remaining[opponent] > 0)
        score -= 45.0f;

    return score;
}

float removal_fragility(const GameState *state, int player) {
    float score = 0.0f;
    int row;
    int col;
    int opponent = other_player(player);

    if (state->removals_remaining[opponent] > 0) {
        for (row = 0; row < ROWS; row++) {
            for (col = 0; col < COLS; col++) {
                if (state->board[row][col] == player &&
                    is_removable_support_piece(state, row, col)) {
                    score -= 2.5f * count_weighted_windows_through_cell(state, player, row, col);
                }
            }
        }
    }

    if (state->removals_remaining[player] > 0) {
        for (row = 0; row < ROWS; row++) {
            for (col = 0; col < COLS; col++) {
                if (state->board[row][col] == opponent &&
                    is_removable_support_piece(state, row, col)) {
                    score += 2.5f * count_weighted_windows_through_cell(state, opponent, row, col);
                }
            }
        }
    }

    return score;
}
