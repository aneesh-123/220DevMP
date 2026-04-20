#include <stdio.h>
#include <string.h>
#include "board.h"

void init_game(GameState *state) {
    memset(state->board, 0, sizeof(state->board));
    state->current_player = PLAYER_X;
    state->removals_remaining[0] = 0;
    state->removals_remaining[PLAYER_X] = 1;
    state->removals_remaining[PLAYER_O] = 1;
    state->is_terminal = 0;
    state->winner = 0;
}

char cell_char(int cell) {
    switch (cell) {
        case PLAYER_X: return 'X';
        case PLAYER_O: return 'O';
        default:       return '.';
    }
}

const char *player_name(int player) {
    switch (player) {
        case PLAYER_X: return "X";
        case PLAYER_O: return "O";
        default:       return "?";
    }
}

void display_board(const GameState *state) {
    printf("\n    ");
    for (int c = 0; c < COLS; c++) {
        printf("%d ", c);
    }
    printf("\n    ");
    for (int c = 0; c < COLS; c++) {
        printf("--");
    }
    printf("\n");

    for (int r = ROWS - 1; r >= 0; r--) {
        printf(" %d| ", r);
        for (int c = 0; c < COLS; c++) {
            printf("%c ", cell_char(state->board[r][c]));
        }
        printf("\n");
    }

    printf("\nTurn: Player %s   |   Removals left  X:%d  O:%d\n",
           player_name(state->current_player),
           state->removals_remaining[PLAYER_X],
           state->removals_remaining[PLAYER_O]);
}
