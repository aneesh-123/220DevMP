#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

/* Board dimensions */
#define ROWS 6
#define COLS 7
#define CONNECT 4
#define MAX_REMOVALS 3

/* Cell values */
#define EMPTY '.'
#define PLAYER_X 'X'
#define PLAYER_O 'O'

/* Move types */
#define MOVE_DROP 0
#define MOVE_REMOVE 1

/* Maximum possible moves per turn:
   7 drops + up to 42 removals = 49 */
#define MAX_MOVES 49

/* ---- Structs ---- */

typedef struct {
    /* board[row][col]: row 0 = BOTTOM, row 5 = TOP */
    char board[ROWS][COLS];
    char current_player;  /* 'X' or 'O' */
    int removals_left_X;
    int removals_left_O;
} GameState;

typedef struct {
    int type;  /* MOVE_DROP or MOVE_REMOVE */
    int col;
    int row;   /* only meaningful for MOVE_REMOVE */
} Move;

/* ---- Functions ---- */

/* Initialize a fresh game state (empty board, X goes first). */
void init_state(GameState *s);

/* Print the board to stdout with column numbers. */
void print_board(const GameState *s);

/* Returns 1 if dropping into col is legal (column not full). */
int is_legal_drop(const GameState *s, int col);

/* Apply a drop into col for the current player. Modifies s in place.
   Returns the row where the piece landed, or -1 if illegal. */
int apply_drop(GameState *s, int col);

/* Returns 1 if removing the piece at (col, row) is legal:
   - cell is occupied
   - current player has removals remaining */
int is_legal_remove(const GameState *s, int col, int row);

/* Apply removal at (col, row), then apply gravity in that column.
   Modifies s in place. Returns 0 on success, -1 if illegal. */
int apply_remove(GameState *s, int col, int row);

/* Check for a winner. Returns 'X', 'O', or '.' (no winner). */
char check_winner(const GameState *s);

/* Returns 1 if the board is full and there is no winner (draw). */
int is_draw(const GameState *s);

/* Generate all legal moves for the current player.
   Writes moves into moves_out (caller provides array of MAX_MOVES).
   Returns the number of moves generated. */
int generate_moves(const GameState *s, Move *moves_out);

/* Switch current_player from X<->O. */
void switch_player(GameState *s);

/* Return how many removals the current player has left. */
int current_removals_left(const GameState *s);

#endif /* ENGINE_H */
