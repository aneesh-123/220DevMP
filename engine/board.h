#ifndef BOARD_H
#define BOARD_H

#define ROWS 6
#define COLS 7

#define EMPTY 0
#define PLAYER_X 1
#define PLAYER_O 2

typedef struct {
    int board[ROWS][COLS];
    int current_player;
    int removals_remaining[3];
    int is_terminal;
    int winner;
} GameState;

void init_game(GameState *state);
void display_board(const GameState *state);
char cell_char(int cell);
const char *player_name(int player);

#endif
