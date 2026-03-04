#include "engine.h"
#include <stdio.h>
#include <string.h>

/* ---- init_state ---- */
void init_state(GameState *s) {
    memset(s->board, EMPTY, sizeof(s->board));
    s->current_player = PLAYER_X;
    s->removals_left_X = MAX_REMOVALS;
    s->removals_left_O = MAX_REMOVALS;
}

/* ---- print_board ---- */
void print_board(const GameState *s) {
    /* Print from top row (5) down to bottom row (0). */
    printf("\n");
    for (int r = ROWS - 1; r >= 0; r--) {
        printf("  %d |", r);
        for (int c = 0; c < COLS; c++) {
            printf(" %c", s->board[r][c]);
        }
        printf("\n");
    }
    printf("    +");
    for (int c = 0; c < COLS; c++) printf("--");
    printf("\n     ");
    for (int c = 0; c < COLS; c++) printf(" %d", c);
    printf("\n\n");
}

/* ---- is_legal_drop ---- */
int is_legal_drop(const GameState *s, int col) {
    if (col < 0 || col >= COLS) return 0;
    /* Column is not full if the top row is empty. */
    return s->board[ROWS - 1][col] == EMPTY;
}

/* ---- apply_drop ---- */
int apply_drop(GameState *s, int col) {
    if (!is_legal_drop(s, col)) return -1;
    /* Find lowest empty row in this column. */
    for (int r = 0; r < ROWS; r++) {
        if (s->board[r][col] == EMPTY) {
            s->board[r][col] = s->current_player;
            return r;
        }
    }
    return -1; /* should not reach here */
}

/* ---- is_legal_remove ---- */
int is_legal_remove(const GameState *s, int col, int row) {
    if (col < 0 || col >= COLS) return 0;
    if (row < 0 || row >= ROWS) return 0;
    if (s->board[row][col] == EMPTY) return 0;
    /* Current player must have removals left. */
    if (s->current_player == PLAYER_X)
        return s->removals_left_X > 0;
    else
        return s->removals_left_O > 0;
}

/* ---- apply_remove ---- */
int apply_remove(GameState *s, int col, int row) {
    if (!is_legal_remove(s, col, row)) return -1;

    /* Decrement removals for current player. */
    if (s->current_player == PLAYER_X)
        s->removals_left_X--;
    else
        s->removals_left_O--;

    /* Remove the piece. */
    s->board[row][col] = EMPTY;

    /* Apply gravity: shift everything above row down within this column. */
    for (int r = row; r < ROWS - 1; r++) {
        s->board[r][col] = s->board[r + 1][col];
    }
    s->board[ROWS - 1][col] = EMPTY;

    return 0;
}

/* ---- check_winner ---- */
char check_winner(const GameState *s) {
    /* Check all possible 4-in-a-row windows. */
    /* Directions: right (0,1), up (1,0), up-right (1,1), up-left (1,-1) */
    static const int dr[] = {0, 1, 1,  1};
    static const int dc[] = {1, 0, 1, -1};

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            char cell = s->board[r][c];
            if (cell == EMPTY) continue;

            for (int dir = 0; dir < 4; dir++) {
                int ok = 1;
                for (int k = 1; k < CONNECT; k++) {
                    int nr = r + dr[dir] * k;
                    int nc = c + dc[dir] * k;
                    if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS ||
                        s->board[nr][nc] != cell) {
                        ok = 0;
                        break;
                    }
                }
                if (ok) return cell;
            }
        }
    }
    return EMPTY;
}

/* ---- is_draw ---- */
int is_draw(const GameState *s) {
    if (check_winner(s) != EMPTY) return 0;
    /* Board is full if every top-row cell is occupied. */
    for (int c = 0; c < COLS; c++) {
        if (s->board[ROWS - 1][c] == EMPTY) return 0;
    }
    return 1;
}

/* ---- switch_player ---- */
void switch_player(GameState *s) {
    s->current_player = (s->current_player == PLAYER_X) ? PLAYER_O : PLAYER_X;
}

/* ---- current_removals_left ---- */
int current_removals_left(const GameState *s) {
    return (s->current_player == PLAYER_X) ?
           s->removals_left_X : s->removals_left_O;
}

/* ---- generate_moves ---- */
int generate_moves(const GameState *s, Move *moves_out) {
    int count = 0;

    /* 1) Drop moves (columns 0..6). */
    for (int c = 0; c < COLS; c++) {
        if (is_legal_drop(s, c)) {
            moves_out[count].type = MOVE_DROP;
            moves_out[count].col = c;
            moves_out[count].row = -1;
            count++;
        }
    }

    /* 2) Removal moves (if current player has removals left). */
    if (current_removals_left(s) > 0) {
        for (int c = 0; c < COLS; c++) {
            for (int r = 0; r < ROWS; r++) {
                if (s->board[r][c] != EMPTY) {
                    moves_out[count].type = MOVE_REMOVE;
                    moves_out[count].col = c;
                    moves_out[count].row = r;
                    count++;
                }
            }
        }
    }

    return count;
}
