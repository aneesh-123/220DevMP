#ifndef BENCH_H
#define BENCH_H

#include "engine.h"

/* Maximum number of board files loadable in one benchmark run. */
#define MAX_BOARDS 64

/* A loaded board position with its source filename. */
typedef struct {
    GameState state;
    char      name[128]; /* filename without path, e.g. "board1.txt" */
} BoardEntry;

/* Load a single board from a file.
   File format (6 lines top-to-bottom, then TURN and REM lines):
     . . . . . . .      (row 5 = top, spaces optional)
     . . . . . . .
     . . O X . . .
     . X O X . . .
     . O X O . . .
     X X O O . . .      (row 0 = bottom)
     TURN X
     REM X=2 O=1

   Returns 0 on success, -1 on error (prints message to stderr). */
int load_board(const char *filepath, BoardEntry *out);

/* Load all board*.txt files from a directory.
   Writes entries into boards_out (caller provides array of MAX_BOARDS).
   Returns the number of boards loaded, or -1 on error. */
int load_boards_from_dir(const char *dirpath, BoardEntry *boards_out);

/* Run the benchmark.
   boards:     array of loaded board positions
   num_boards: how many boards
   depth:      fixed search depth
   modes:      array of mode indices to test (e.g. {0,1,2})
   num_modes:  how many modes
   csv_path:   if non-NULL, write CSV output to this file */
void run_benchmark(const BoardEntry *boards, int num_boards,
                   int depth, const int *modes, int num_modes,
                   const char *csv_path);

#endif /* BENCH_H */
