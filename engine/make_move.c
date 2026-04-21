#include "make_move.h"
#include "rules.h"
#include <limits.h>
#include <string.h>

#define MAX_DEPTH 6

typedef struct {
    int move_type;
    int row;
    int col;
    int score;
} ScoredMove;

static int evaluate(const GameState *state, int maximizing_player) {
    int opponent = (maximizing_player == PLAYER_X) ? PLAYER_O : PLAYER_X;

    if (check_win(state, maximizing_player)) return 10000;
    if (check_win(state, opponent)) return -10000;
    if (is_board_full(state)) return 0;

    int score = 0;

    // Evaluate positioning and threats
    for (int c = 0; c < COLS; c++) {
        int h = column_height(state, c);
        if (h < ROWS) {
            int cell = state->board[h][c];
            if (cell == maximizing_player) score += 10;
            else if (cell == opponent) score -= 10;
        }
    }

    // Count potential winning patterns
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (state->board[r][c] != EMPTY) {
                int player_cell = state->board[r][c];
                int delta = (player_cell == maximizing_player) ? 1 : -1;

                // Horizontal
                if (c + 3 < COLS) {
                    int count = 0, empty = 0;
                    for (int i = 0; i < 4; i++) {
                        if (state->board[r][c+i] == player_cell) count++;
                        else if (state->board[r][c+i] == EMPTY) empty++;
                    }
                    if (empty > 0) score += delta * count * 2;
                }

                // Vertical
                if (r + 3 < ROWS) {
                    int count = 0, empty = 0;
                    for (int i = 0; i < 4; i++) {
                        if (state->board[r+i][c] == player_cell) count++;
                        else if (state->board[r+i][c] == EMPTY) empty++;
                    }
                    if (empty > 0) score += delta * count * 2;
                }

                // Diagonal down-right
                if (r + 3 < ROWS && c + 3 < COLS) {
                    int count = 0, empty = 0;
                    for (int i = 0; i < 4; i++) {
                        if (state->board[r+i][c+i] == player_cell) count++;
                        else if (state->board[r+i][c+i] == EMPTY) empty++;
                    }
                    if (empty > 0) score += delta * count * 2;
                }

                // Diagonal down-left
                if (r + 3 < ROWS && c - 3 >= 0) {
                    int count = 0, empty = 0;
                    for (int i = 0; i < 4; i++) {
                        if (state->board[r+i][c-i] == player_cell) count++;
                        else if (state->board[r+i][c-i] == EMPTY) empty++;
                    }
                    if (empty > 0) score += delta * count * 2;
                }
            }
        }
    }

    return score;
}

static int minimax(GameState *state, int depth, int maximizing,
                   int alpha, int beta, int maximizing_player) {
    if (depth == 0 || state->is_terminal) {
        return evaluate(state, maximizing_player);
    }

    int current_player = state->current_player;

    if (maximizing) {
        int max_eval = INT_MIN;

        // Try placements
        for (int col = 0; col < COLS; col++) {
            Move m = { .type = MOVE_PLACEMENT, .column = col, .row = 0, .col = 0 };
            if (is_legal_move(state, m)) {
                GameState copy;
                memcpy(&copy, state, sizeof(GameState));
                apply_move(&copy, m);
                int eval = minimax(&copy, depth - 1, 0, alpha, beta, maximizing_player);
                max_eval = (eval > max_eval) ? eval : max_eval;
                alpha = (eval > alpha) ? eval : alpha;
                if (beta <= alpha) break;
            }
        }

        // Try removals if player has removals remaining
        if (state->removals_remaining[current_player] > 0) {
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    Move m = { .type = MOVE_REMOVAL, .column = 0, .row = r, .col = c };
                    if (is_legal_move(state, m)) {
                        GameState copy;
                        memcpy(&copy, state, sizeof(GameState));
                        apply_move(&copy, m);
                        int eval = minimax(&copy, depth - 1, 0, alpha, beta, maximizing_player);
                        max_eval = (eval > max_eval) ? eval : max_eval;
                        alpha = (eval > alpha) ? eval : alpha;
                        if (beta <= alpha) break;
                    }
                }
                if (beta <= alpha) break;
            }
        }

        return max_eval;
    } else {
        int min_eval = INT_MAX;

        // Try placements
        for (int col = 0; col < COLS; col++) {
            Move m = { .type = MOVE_PLACEMENT, .column = col, .row = 0, .col = 0 };
            if (is_legal_move(state, m)) {
                GameState copy;
                memcpy(&copy, state, sizeof(GameState));
                apply_move(&copy, m);
                int eval = minimax(&copy, depth - 1, 1, alpha, beta, maximizing_player);
                min_eval = (eval < min_eval) ? eval : min_eval;
                beta = (eval < beta) ? eval : beta;
                if (beta <= alpha) break;
            }
        }

        // Try removals if player has removals remaining
        if (state->removals_remaining[current_player] > 0) {
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    Move m = { .type = MOVE_REMOVAL, .column = 0, .row = r, .col = c };
                    if (is_legal_move(state, m)) {
                        GameState copy;
                        memcpy(&copy, state, sizeof(GameState));
                        apply_move(&copy, m);
                        int eval = minimax(&copy, depth - 1, 1, alpha, beta, maximizing_player);
                        min_eval = (eval < min_eval) ? eval : min_eval;
                        beta = (eval < beta) ? eval : beta;
                        if (beta <= alpha) break;
                    }
                }
                if (beta <= alpha) break;
            }
        }

        return min_eval;
    }
}

void make_move(GameState *state, int player,
               int *move_type, int *row, int *column) {
    int best_score = INT_MIN;
    int best_move_type = MOVE_PLACEMENT;
    int best_row = 0;
    int best_col = 0;

    // Try all placement moves
    for (int col = 0; col < COLS; col++) {
        Move m = { .type = MOVE_PLACEMENT, .column = col, .row = 0, .col = 0 };
        if (is_legal_move(state, m)) {
            GameState copy;
            memcpy(&copy, state, sizeof(GameState));
            apply_move(&copy, m);
            int score = minimax(&copy, MAX_DEPTH - 1, 0, INT_MIN, INT_MAX, player);

            if (score > best_score) {
                best_score = score;
                best_move_type = MOVE_PLACEMENT;
                best_col = col;
                best_row = 0;
            }
        }
    }

    // Try all removal moves if player has removals left
    if (state->removals_remaining[player] > 0) {
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                Move m = { .type = MOVE_REMOVAL, .column = 0, .row = r, .col = c };
                if (is_legal_move(state, m)) {
                    GameState copy;
                    memcpy(&copy, state, sizeof(GameState));
                    apply_move(&copy, m);
                    int score = minimax(&copy, MAX_DEPTH - 1, 0, INT_MIN, INT_MAX, player);

                    if (score > best_score) {
                        best_score = score;
                        best_move_type = MOVE_REMOVAL;
                        best_row = r;
                        best_col = c;
                    }
                }
            }
        }
    }

    *move_type = best_move_type;
    *row = best_row;
    *column = best_col;
}
