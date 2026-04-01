#ifndef TEST_OPPONENT_UTILS_H
#define TEST_OPPONENT_UTILS_H

#include "../engine/board.h"

typedef struct {
    float center_weight;
    float edge_weight;
    float material_weight;
    float mobility_weight;
    float support_weight;
    float open_lane_weight;
    float friendly_two_weight;
    float friendly_three_weight;
    float playable_three_weight;
    float enemy_two_weight;
    float enemy_three_weight;
    float enemy_playable_three_weight;
    float vertical_weight;
    float diagonal_weight;
    float fork_weight;
    float removal_weight;
} OpponentStyle;

typedef struct {
    float friendly_two;
    float friendly_three;
    float friendly_playable_three;
    float enemy_two;
    float enemy_three;
    float enemy_playable_three;
    float vertical;
    float diagonal;
    float forks;
    float open_lanes;
} PatternStats;

static inline int abs_int(int value) {
    return value < 0 ? -value : value;
}

static inline float terminal_score(const GameState *state, int player) {
    if (!state->is_terminal)
        return 0.0f;

    if (state->winner == player)
        return 100000.0f;
    if (state->winner == WINNER_NONE)
        return 0.0f;
    return -100000.0f;
}

static inline float count_center_control(const GameState *state, int player) {
    float score = 0.0f;
    int row, col;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int cell = state->board[row][col];
            int distance = abs_int(col - 3);
            float weight = (float)(3 - distance);

            if (cell == player)
                score += weight;
            else if (cell != CELL_EMPTY)
                score -= weight;
        }
    }

    return score;
}

static inline float count_edge_pressure(const GameState *state, int player) {
    float score = 0.0f;
    int row;

    for (row = 0; row < ROWS; row++) {
        if (state->board[row][0] == player)
            score += 1.0f;
        else if (state->board[row][0] != CELL_EMPTY)
            score -= 1.0f;

        if (state->board[row][6] == player)
            score += 1.0f;
        else if (state->board[row][6] != CELL_EMPTY)
            score -= 1.0f;
    }

    return score;
}

static inline float count_material(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(gamestate_count_pieces(state, player)
                 - gamestate_count_pieces(state, opponent));
}

static inline int is_playable_cell(const GameState *state, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS)
        return 0;
    if (state->board[row][col] != CELL_EMPTY)
        return 0;
    return row == ROWS - 1 || state->board[row + 1][col] != CELL_EMPTY;
}

static inline float count_mobility(const GameState *state, int player) {
    int opponent = 1 - player;
    int col;
    float score = 0.0f;

    for (col = 0; col < COLS; col++) {
        if (!gamestate_is_column_full(state, col)) {
            score += 1.0f;
            if (col == 3)
                score += 0.5f;
            if (col == 2 || col == 4)
                score += 0.25f;
        }
    }

    if (state->removals_remaining[player] > 0)
        score += 1.5f;
    if (state->removals_remaining[opponent] > 0)
        score -= 1.5f;

    return score;
}

static inline float count_support_strength(const GameState *state, int player) {
    float score = 0.0f;
    int row, col;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            int cell = state->board[row][col];
            float base;

            if (cell == CELL_EMPTY)
                continue;

            base = (row == ROWS - 1) ? 1.2f : 0.6f;
            if (col == 3)
                base += 0.8f;
            else if (col == 2 || col == 4)
                base += 0.4f;

            if (cell == player)
                score += base;
            else
                score -= base;
        }
    }

    return score;
}

static inline void score_window(const GameState *state,
                                int player,
                                int row,
                                int col,
                                int d_row,
                                int d_col,
                                PatternStats *stats) {
    int opponent = 1 - player;
    int cells[4];
    int i;
    int friendly = 0;
    int enemy = 0;
    int empty = 0;
    int playable_empties = 0;

    for (i = 0; i < 4; i++) {
        int value = state->board[row + d_row * i][col + d_col * i];
        cells[i] = value;

        if (value == player)
            friendly++;
        else if (value == opponent)
            enemy++;
        else
            empty++;
    }

    if (friendly > 0 && enemy > 0)
        return;

    if (friendly > 0) {
        for (i = 0; i < 4; i++) {
            int r = row + d_row * i;
            int c = col + d_col * i;
            if (cells[i] == CELL_EMPTY && is_playable_cell(state, r, c))
                playable_empties++;
        }

        if (friendly == 2 && empty == 2)
            stats->friendly_two += 1.0f;
        if (friendly == 3 && empty == 1) {
            stats->friendly_three += 1.0f;
            if (playable_empties > 0)
                stats->friendly_playable_three += 1.0f;
            if (playable_empties > 1)
                stats->forks += 1.0f;
        }
        if (d_col == 0)
            stats->vertical += (float)(friendly * friendly);
        if (d_row != 0 && d_col != 0)
            stats->diagonal += (float)(friendly * friendly);
        if (enemy == 0)
            stats->open_lanes += (float)(friendly * friendly + empty * 0.25f);
    } else if (enemy > 0) {
        if (enemy == 2 && empty == 2)
            stats->enemy_two += 1.0f;
        if (enemy == 3 && empty == 1) {
            stats->enemy_three += 1.0f;
            if (playable_empties > 0)
                stats->enemy_playable_three += 1.0f;
        }
    }
}

static inline void count_pattern_score(const GameState *state,
                                       int player,
                                       PatternStats *stats) {
    int row, col;

    stats->friendly_two = 0.0f;
    stats->friendly_three = 0.0f;
    stats->friendly_playable_three = 0.0f;
    stats->enemy_two = 0.0f;
    stats->enemy_three = 0.0f;
    stats->enemy_playable_three = 0.0f;
    stats->vertical = 0.0f;
    stats->diagonal = 0.0f;
    stats->forks = 0.0f;
    stats->open_lanes = 0.0f;

    for (row = 0; row < ROWS; row++) {
        for (col = 0; col + 3 < COLS; col++) {
            score_window(state, player, row, col, 0, 1, stats);
        }
    }

    for (row = 0; row + 3 < ROWS; row++) {
        for (col = 0; col < COLS; col++) {
            score_window(state, player, row, col, 1, 0, stats);
        }
    }

    for (row = 0; row + 3 < ROWS; row++) {
        for (col = 0; col + 3 < COLS; col++) {
            score_window(state, player, row, col, 1, 1, stats);
        }
    }

    for (row = 3; row < ROWS; row++) {
        for (col = 0; col + 3 < COLS; col++) {
            score_window(state, player, row, col, -1, 1, stats);
        }
    }
}

static inline float evaluate_style(const GameState *state,
                                   int player,
                                   OpponentStyle style) {
    PatternStats stats;
    float score = terminal_score(state, player);

    if (score != 0.0f)
        return score;

    count_pattern_score(state, player, &stats);

    score += style.center_weight * count_center_control(state, player);
    score += style.edge_weight * count_edge_pressure(state, player);
    score += style.material_weight * count_material(state, player);
    score += style.mobility_weight * count_mobility(state, player);
    score += style.support_weight * count_support_strength(state, player);
    score += style.open_lane_weight * stats.open_lanes;
    score += style.friendly_two_weight * stats.friendly_two;
    score += style.friendly_three_weight * stats.friendly_three;
    score += style.playable_three_weight * stats.friendly_playable_three;
    score -= style.enemy_two_weight * stats.enemy_two;
    score -= style.enemy_three_weight * stats.enemy_three;
    score -= style.enemy_playable_three_weight * stats.enemy_playable_three;
    score += style.vertical_weight * stats.vertical;
    score += style.diagonal_weight * stats.diagonal;
    score += style.fork_weight * stats.forks;
    score += style.removal_weight *
             (float)(state->removals_remaining[player] - state->removals_remaining[1 - player]);

    return score;
}

#endif
