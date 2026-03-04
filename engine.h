#ifndef ENGINE_H
#define ENGINE_H

/*
 * engine.h — Modified Connect 4 game engine
 *
 * Board layout:
 *   board[row][col] where row 0 = BOTTOM, row 5 = TOP.
 *   Cell values: '.' (empty), 'X', 'O'.
 *
 * Rules:
 *   - 7 columns (0..6), 6 rows (0..5).
 *   - Each turn: DROP into a column, or REMOVE any occupied cell
 *     (if the current player has removals remaining; max 3 per player).
 *   - After a removal, gravity pulls pieces above downward in that column.
 *   - 4 in a row (horizontal, vertical, diagonal) wins immediately.
 */

#define ROWS 6
#define COLS 7
#define CONNECT 4
#define MAX_REMOVALS 3

/* Cell values */
#define EMPTY '.'
#define PLAYER_X 'X'
#define PLAYER_O 'O'

/* Move types */
#define MOVE_DROP   0
#define MOVE_REMOVE 1

/* Maximum legal moves per turn:
 * 7 drops + up to 42 removals = 49 */
#define MAX_MOVES 49

/* ---- Structs ---- */

typedef struct {
    char board[ROWS][COLS];  /* board[row][col]; row 0 = bottom */
    char turn;               /* 'X' or 'O' — who moves next */
    int  rem_x;              /* removals remaining for X (0..3) */
    int  rem_o;              /* removals remaining for O (0..3) */
} GameState;

typedef struct {
    int type;   /* MOVE_DROP or MOVE_REMOVE */
    int col;    /* column (0..6) */
    int row;    /* row (only meaningful for MOVE_REMOVE) */
} Move;

/* ---- Functions ---- */

/* Set state to an empty board, X to move, 3 removals each. */
void state_init(GameState *s);

/* Deep-copy src into dst. */
void state_copy(GameState *dst, const GameState *src);

/* Print the board to stdout (top row first, with row/col labels). */
void state_print(const GameState *s);

/* Returns 1 if dropping into col is legal (column not full). */
int is_legal_drop(const GameState *s, int col);

/* Apply a drop for the current player into col.
 * Returns the row where the piece landed, or -1 if illegal. */
int apply_drop(GameState *s, int col);

/* Returns 1 if removing the piece at (col, row) is legal:
 *   - cell is occupied
 *   - current player has removals remaining */
int is_legal_remove(const GameState *s, int col, int row);

/* Apply removal at (col, row), then apply gravity in that column.
 * Returns 0 on success, -1 if illegal. */
int apply_remove(GameState *s, int col, int row);

/* Check for a winner.
 * Returns 'X', 'O', or '.' (no winner yet). */
char check_winner(const GameState *s);

/* Returns 1 if the board is completely full (and no winner). */
int is_board_full(const GameState *s);

/* Switch the current player: X <-> O. */
void switch_turn(GameState *s);

/* How many removals does the current player have left? */
int current_removals(const GameState *s);

/* Generate all legal moves for the current player.
 * Writes into moves_out (caller provides array of MAX_MOVES).
 * Returns the number of moves generated.
 *
 * Order: drops col 0..6 first, then removals in (col, row) order
 * (col 0..6, row 0..5) — this gives deterministic tie-breaking. */
int generate_moves(const GameState *s, Move *moves_out);

/* Format a move as a string into buf (caller provides >=32 bytes).
 * Returns buf for convenience. */
char *move_to_str(const Move *m, char *buf, int buflen);

#endif /* ENGINE_H */
