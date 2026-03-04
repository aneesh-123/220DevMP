#include "engine.h"
#include <stdio.h>
#include <string.h>

/* ---- state_init ---- */
void state_init(GameState *s) {
    memset(s->board, EMPTY, sizeof(s->board));
    s->turn  = PLAYER_X;
    s->rem_x = MAX_REMOVALS;
    s->rem_o = MAX_REMOVALS;
}

/* ---- state_copy ---- */
void state_copy(GameState *dst, const GameState *src) {
    *dst = *src;
}

/* ---- state_print ---- */
void state_print(const GameState *s) {
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
    printf("\n");
}

/* ---- is_legal_drop ---- */
int is_legal_drop(const GameState *s, int col) {
    if (col < 0 || col >= COLS) return 0;
    return s->board[ROWS - 1][col] == EMPTY;  /* top row empty = space */
}

/* ---- apply_drop ---- */
int apply_drop(GameState *s, int col) {
    if (!is_legal_drop(s, col)) return -1;
    for (int r = 0; r < ROWS; r++) {
        if (s->board[r][col] == EMPTY) {
            s->board[r][col] = s->turn;
            return r;
        }
    }
    return -1;  /* unreachable */
}

/* ---- is_legal_remove ---- */
int is_legal_remove(const GameState *s, int col, int row) {
    if (col < 0 || col >= COLS) return 0;
    if (row < 0 || row >= ROWS) return 0;
    if (s->board[row][col] == EMPTY) return 0;
    /* Current player must have removals remaining. */
    if (s->turn == PLAYER_X) return s->rem_x > 0;
    return s->rem_o > 0;
}

/* ---- apply_remove ---- */
int apply_remove(GameState *s, int col, int row) {
    if (!is_legal_remove(s, col, row)) return -1;

    /* Decrement removals for the current player. */
    if (s->turn == PLAYER_X) s->rem_x--;
    else                     s->rem_o--;

    /* Remove the piece. */
    s->board[row][col] = EMPTY;

    /* Apply gravity: shift pieces above the removed cell downward. */
    for (int r = row; r < ROWS - 1; r++) {
        s->board[r][col] = s->board[r + 1][col];
    }
    s->board[ROWS - 1][col] = EMPTY;

    return 0;
}

/* ---- check_winner ---- */
char check_winner(const GameState *s) {
    /* Four search directions: right, up, up-right diagonal, up-left diagonal. */
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

/* ---- is_board_full ---- */
int is_board_full(const GameState *s) {
    for (int c = 0; c < COLS; c++) {
        if (s->board[ROWS - 1][c] == EMPTY) return 0;
    }
    return 1;
}

/* ---- switch_turn ---- */
void switch_turn(GameState *s) {
    s->turn = (s->turn == PLAYER_X) ? PLAYER_O : PLAYER_X;
}

/* ---- current_removals ---- */
int current_removals(const GameState *s) {
    return (s->turn == PLAYER_X) ? s->rem_x : s->rem_o;
}

/* ---- generate_moves ---- */
int generate_moves(const GameState *s, Move *moves_out) {
    int count = 0;

    /* 1) Drop moves: columns 0..6, in order. */
    for (int c = 0; c < COLS; c++) {
        if (is_legal_drop(s, c)) {
            moves_out[count].type = MOVE_DROP;
            moves_out[count].col  = c;
            moves_out[count].row  = -1;
            count++;
        }
    }

    /* 2) Removal moves: only if current player has removals left.
     *    Enumerate in (col, row) order for deterministic tie-breaking. */
    if (current_removals(s) > 0) {
        for (int c = 0; c < COLS; c++) {
            for (int r = 0; r < ROWS; r++) {
                if (s->board[r][c] != EMPTY) {
                    moves_out[count].type = MOVE_REMOVE;
                    moves_out[count].col  = c;
                    moves_out[count].row  = r;
                    count++;
                }
            }
        }
    }

    return count;
}

/* ---- move_to_str ---- */
char *move_to_str(const Move *m, char *buf, int buflen) {
    if (m->type == MOVE_DROP)
        snprintf(buf, buflen, "DROP col=%d", m->col);
    else
        snprintf(buf, buflen, "REMOVE col=%d row=%d", m->col, m->row);
    return buf;
}
