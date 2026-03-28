#include "rules.h"

int get_legal_moves(const GameState *state, Move *moves_out) {
    int count = 0;
    int col, row;

    /* Placement moves: any non-full column */
    for (col = 0; col < COLS; col++) {
        if (!gamestate_is_column_full(state, col)) {
            moves_out[count++] = move_placement(col);
        }
    }

    /* Removal moves: any occupied cell if player has removals left */
    if (state->removals_remaining[state->current_player] > 0) {
        for (row = 0; row < ROWS; row++) {
            for (col = 0; col < COLS; col++) {
                if (gamestate_is_cell_occupied(state, row, col)) {
                    moves_out[count++] = move_removal(row, col);
                }
            }
        }
    }

    return count;
}

static void apply_placement(GameState *state, Move move) {
    int col = move.column;
    int row;
    for (row = ROWS - 1; row >= 0; row--) {
        if (state->board[row][col] == CELL_EMPTY) {
            state->board[row][col] = state->current_player;
            return;
        }
    }
}

static void apply_removal(GameState *state, Move move) {
    int row = move.row;
    int col = move.col;
    int src;

    /* Remove the piece */
    state->board[row][col] = CELL_EMPTY;

    /* Apply gravity: shift pieces above the gap downward */
    for (src = row - 1; src >= 0; src--) {
        if (state->board[src][col] != CELL_EMPTY) {
            state->board[src + 1][col] = state->board[src][col];
            state->board[src][col] = CELL_EMPTY;
        }
    }

    /* Decrement removal count */
    state->removals_remaining[state->current_player]--;
}

int check_win(const GameState *state) {
    int player = state->current_player;
    int row, col, i;

    /* Horizontal */
    for (row = 0; row < ROWS; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            int ok = 1;
            for (i = 0; i < 4; i++)
                if (state->board[row][col + i] != player) { ok = 0; break; }
            if (ok) return 1;
        }
    }

    /* Vertical */
    for (col = 0; col < COLS; col++) {
        for (row = 0; row <= ROWS - 4; row++) {
            int ok = 1;
            for (i = 0; i < 4; i++)
                if (state->board[row + i][col] != player) { ok = 0; break; }
            if (ok) return 1;
        }
    }

    /* Diagonal (top-left to bottom-right) */
    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 0; col <= COLS - 4; col++) {
            int ok = 1;
            for (i = 0; i < 4; i++)
                if (state->board[row + i][col + i] != player) { ok = 0; break; }
            if (ok) return 1;
        }
    }

    /* Diagonal (top-right to bottom-left) */
    for (row = 0; row <= ROWS - 4; row++) {
        for (col = 3; col < COLS; col++) {
            int ok = 1;
            for (i = 0; i < 4; i++)
                if (state->board[row + i][col - i] != player) { ok = 0; break; }
            if (ok) return 1;
        }
    }

    return 0;
}

int is_board_full(const GameState *state) {
    int col;
    for (col = 0; col < COLS; col++)
        if (!gamestate_is_column_full(state, col))
            return 0;
    return 1;
}

GameState apply_move(const GameState *state, Move move) {
    GameState new_state;
    gamestate_copy(state, &new_state);

    if (move.type == MOVE_PLACEMENT)
        apply_placement(&new_state, move);
    else
        apply_removal(&new_state, move);

    /* Check for win */
    if (check_win(&new_state)) {
        new_state.winner = state->current_player;
        new_state.is_terminal = 1;
    } else if (is_board_full(&new_state)) {
        new_state.is_terminal = 1;
        new_state.winner = WINNER_NONE;
    }

    /* Switch player if game is not terminal */
    if (!new_state.is_terminal)
        new_state.current_player = 1 - new_state.current_player;

    return new_state;
}
