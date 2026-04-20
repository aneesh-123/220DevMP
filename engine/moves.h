#ifndef MOVES_H
#define MOVES_H

typedef enum {
    MOVE_PLACEMENT,
    MOVE_REMOVAL
} MoveType;

typedef struct {
    MoveType type;
    int column;
    int row;
    int col;
} Move;

#endif
