#ifndef AI_H
#define AI_H

#include "engine.h"
#include <stdint.h>

/* Score bounds */
#define SCORE_INF  1000000
#define SCORE_WIN  100000

/* Transposition table size (power of 2 for fast masking). */
#define TT_SIZE (1 << 20)  /* ~1 million entries */
#define TT_MASK (TT_SIZE - 1)

/* Transposition table bound types */
#define TT_EXACT 0
#define TT_LOWER 1  /* alpha cutoff: true value >= stored value */
#define TT_UPPER 2  /* beta cutoff:  true value <= stored value */

/* ---- Transposition table entry ---- */
typedef struct {
    uint64_t key;    /* Full Zobrist hash for verification */
    int      depth;  /* Depth at which this was stored */
    int      score;  /* Evaluated score */
    int      bound;  /* TT_EXACT, TT_LOWER, or TT_UPPER */
    int      valid;  /* 1 if this entry is occupied */
} TTEntry;

/* ---- AI search statistics (reset per move) ---- */
typedef struct {
    long long nodes_expanded;
    long long tt_probes;
    long long tt_hits;
} AIStats;

/* ---- Zobrist table (initialized once) ---- */
/* zobrist_pieces[row][col][piece_index]: piece_index 0=X, 1=O */
extern uint64_t zobrist_pieces[ROWS][COLS][2];
extern uint64_t zobrist_player;  /* XOR when it's O's turn */
extern uint64_t zobrist_removals[2][MAX_REMOVALS + 1]; /* [player_idx][count] */

/* Initialize Zobrist random keys (call once at startup). */
void ai_init_zobrist(void);

/* Compute full Zobrist hash for a state (used initially; incremental
   updates are possible but not implemented in this POC). */
uint64_t zobrist_hash(const GameState *s);

/* Clear the transposition table. */
void tt_clear(void);

/* Choose the best move for the current player.
   mode: 0 = baseline minimax, 1 = alpha-beta, 2 = alpha-beta + TT
   depth: fixed search depth
   stats: output statistics (nodes, tt_probes, tt_hits)
   Returns the chosen Move. */
Move ai_choose_move(const GameState *s, int mode, int depth, AIStats *stats);

/* Heuristic evaluation of the board from X's perspective.
   Positive = good for X, negative = good for O. */
int evaluate(const GameState *s);

#endif /* AI_H */
