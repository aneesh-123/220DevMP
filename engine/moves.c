#include <stdio.h>
#include "moves.h"

Move move_placement(int column) {
    Move m;
    m.type = MOVE_PLACEMENT;
    m.column = column;
    m.row = -1;
    m.col = -1;
    return m;
}

Move move_removal(int row, int col) {
    Move m;
    m.type = MOVE_REMOVAL;
    m.column = -1;
    m.row = row;
    m.col = col;
    return m;
}

void move_to_string(Move move, char *buf) {
    if (move.type == MOVE_PLACEMENT)
        sprintf(buf, "Place in column %d", move.column);
    else
        sprintf(buf, "Remove row %d col %d", move.row, move.col);
}

int move_equals(Move a, Move b) {
    if (a.type != b.type) return 0;
    if (a.type == MOVE_PLACEMENT)
        return a.column == b.column;
    return a.row == b.row && a.col == b.col;
}
