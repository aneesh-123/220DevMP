#ifndef BOARD_H
#define BOARD_H

#define ROWS 6
#define COLS 7
#define CELL_EMPTY -1
#define CELL_P0 0
#define CELL_P1 1

#define WINNER_NONE -1

typedef struct {
    int board[ROWS][COLS];
    int current_player;
    int removals_remaining[2];
    int is_terminal;
    int winner; /* CELL_P0, CELL_P1, or WINNER_NONE */
} GameState;

/* Initialize a blank game state. */
void gamestate_init(GameState *state);

/* Deep copy src into dst. */
void gamestate_copy(const GameState *src, GameState *dst);

/* Get the value at (row, col). Returns CELL_EMPTY if out of bounds. */
int gamestate_get_cell(const GameState *state, int row, int col);

/* Set the value at (row, col). */
void gamestate_set_cell(GameState *state, int row, int col, int value);

/* Returns 1 if column is full, 0 otherwise. */
int gamestate_is_column_full(const GameState *state, int col);

/* Count pieces in a column. */
int gamestate_get_column_height(const GameState *state, int col);

/* Returns 1 if cell is occupied, 0 otherwise. */
int gamestate_is_cell_occupied(const GameState *state, int row, int col);

/* Count pieces belonging to player. */
int gamestate_count_pieces(const GameState *state, int player);

/* Print the board to stdout. */
void gamestate_display(const GameState *state);

#endif
