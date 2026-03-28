#include <stdio.h>
#include "board.h"

void gamestate_init(GameState *state) {
    int r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            state->board[r][c] = CELL_EMPTY;
    state->current_player = CELL_P0;
    state->removals_remaining[0] = 1;
    state->removals_remaining[1] = 1;
    state->is_terminal = 0;
    state->winner = WINNER_NONE;
}

void gamestate_copy(const GameState *src, GameState *dst) {
    *dst = *src;
}

int gamestate_get_cell(const GameState *state, int row, int col) {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS)
        return CELL_EMPTY;
    return state->board[row][col];
}

void gamestate_set_cell(GameState *state, int row, int col, int value) {
    if (row >= 0 && row < ROWS && col >= 0 && col < COLS)
        state->board[row][col] = value;
}

int gamestate_is_column_full(const GameState *state, int col) {
    int r;
    for (r = 0; r < ROWS; r++)
        if (state->board[r][col] == CELL_EMPTY)
            return 0;
    return 1;
}

int gamestate_get_column_height(const GameState *state, int col) {
    int count = 0, r;
    for (r = 0; r < ROWS; r++)
        if (state->board[r][col] != CELL_EMPTY)
            count++;
    return count;
}

int gamestate_is_cell_occupied(const GameState *state, int row, int col) {
    return gamestate_get_cell(state, row, col) != CELL_EMPTY;
}

int gamestate_count_pieces(const GameState *state, int player) {
    int count = 0, r, c;
    for (r = 0; r < ROWS; r++)
        for (c = 0; c < COLS; c++)
            if (state->board[r][c] == player)
                count++;
    return count;
}

void gamestate_display(const GameState *state) {
    int r, c;
    for (r = 0; r < ROWS; r++) {
        printf("%d |", ROWS - 1 - r);
        for (c = 0; c < COLS; c++) {
            int cell = state->board[r][c];
            if (cell == CELL_EMPTY)
                printf(" .|");
            else if (cell == CELL_P0)
                printf(" O|");
            else
                printf(" X|");
        }
        printf("\n");
    }
    printf("  +--+--+--+--+--+--+--+\n");
    printf("  | 0| 1| 2| 3| 4| 5| 6|\n\n");
    printf("Current player: %c\n", state->current_player == CELL_P0 ? 'O' : 'X');
    printf("Removals remaining: [%d, %d]\n",
           state->removals_remaining[0], state->removals_remaining[1]);

    if (state->is_terminal) {
        if (state->winner != WINNER_NONE)
            printf("GAME OVER: Player %c wins!\n",
                   state->winner == CELL_P0 ? 'O' : 'X');
        else
            printf("GAME OVER: Board is full.\n");
    }
}
