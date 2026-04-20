#include "rules.h"

int column_height(const GameState *state, int col) {
    int h = 0;
    for (int r = 0; r < ROWS; r++) {
        if (state->board[r][col] != EMPTY) h++;
    }
    return h;
}

int is_legal_move(const GameState *state, Move move) {
    if (state->is_terminal) return 0;

    if (move.type == MOVE_PLACEMENT) {
        if (move.column < 0 || move.column >= COLS) return 0;
        return state->board[ROWS - 1][move.column] == EMPTY;
    }

    if (move.type == MOVE_REMOVAL) {
        if (state->removals_remaining[state->current_player] <= 0) return 0;
        if (move.row < 0 || move.row >= ROWS) return 0;
        if (move.col < 0 || move.col >= COLS) return 0;
        return state->board[move.row][move.col] != EMPTY;
    }

    return 0;
}

int check_win(const GameState *state, int player) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (state->board[r][c] == player &&
                state->board[r][c+1] == player &&
                state->board[r][c+2] == player &&
                state->board[r][c+3] == player) return 1;
        }
    }

    for (int c = 0; c < COLS; c++) {
        for (int r = 0; r <= ROWS - 4; r++) {
            if (state->board[r][c] == player &&
                state->board[r+1][c] == player &&
                state->board[r+2][c] == player &&
                state->board[r+3][c] == player) return 1;
        }
    }

    for (int r = 0; r <= ROWS - 4; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (state->board[r][c] == player &&
                state->board[r+1][c+1] == player &&
                state->board[r+2][c+2] == player &&
                state->board[r+3][c+3] == player) return 1;
        }
    }

    for (int r = 3; r < ROWS; r++) {
        for (int c = 0; c <= COLS - 4; c++) {
            if (state->board[r][c] == player &&
                state->board[r-1][c+1] == player &&
                state->board[r-2][c+2] == player &&
                state->board[r-3][c+3] == player) return 1;
        }
    }

    return 0;
}

int is_board_full(const GameState *state) {
    for (int c = 0; c < COLS; c++) {
        if (state->board[ROWS - 1][c] == EMPTY) return 0;
    }
    return 1;
}

static void apply_gravity_column(GameState *state, int col) {
    int write = 0;
    for (int r = 0; r < ROWS; r++) {
        if (state->board[r][col] != EMPTY) {
            int piece = state->board[r][col];
            state->board[r][col] = EMPTY;
            state->board[write][col] = piece;
            write++;
        }
    }
}

void apply_move(GameState *state, Move move) {
    int mover = state->current_player;

    if (move.type == MOVE_PLACEMENT) {
        for (int r = 0; r < ROWS; r++) {
            if (state->board[r][move.column] == EMPTY) {
                state->board[r][move.column] = mover;
                break;
            }
        }
    } else {
        state->board[move.row][move.col] = EMPTY;
        apply_gravity_column(state, move.col);
        state->removals_remaining[mover]--;
    }

    int opponent = (mover == PLAYER_X) ? PLAYER_O : PLAYER_X;
    if (check_win(state, mover)) {
        state->is_terminal = 1;
        state->winner = mover;
    } else if (check_win(state, opponent)) {
        state->is_terminal = 1;
        state->winner = opponent;
    } else if (is_board_full(state)) {
        state->is_terminal = 1;
        state->winner = 0;
    }

    state->current_player = opponent;
}
