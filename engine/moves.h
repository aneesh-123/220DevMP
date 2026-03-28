#ifndef MOVES_H
#define MOVES_H

typedef enum {
    MOVE_PLACEMENT,
    MOVE_REMOVAL
} MoveType;

typedef struct {
    MoveType type;
    int column; /* used for placement */
    int row;    /* used for removal */
    int col;    /* used for removal */
} Move;

/* Create a placement move. */
Move move_placement(int column);

/* Create a removal move. */
Move move_removal(int row, int col);

/* Write a human-readable string into buf (must be >= 64 bytes). */
void move_to_string(Move move, char *buf);

/* Returns 1 if two moves are equal. */
int move_equals(Move a, Move b);

#endif
