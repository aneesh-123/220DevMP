/*
 * Terminal-only opponent: returns a large bonus/penalty for wins and losses,
 * and exactly 0 for every non-terminal position.
 *
 * Because all non-terminal states score identically, minimax has no preference
 * between moves and will always pick the first legal move (leftmost column).
 * This makes it a useful baseline: any evaluator with real heuristics should
 * beat it consistently.
 */

#include "../engine/board.h"

float terminal_only_evaluate(const GameState *state, int player) {
    if (!state->is_terminal)
        return 0.0f;

    if (state->winner == player)
        return 10000.0f;
    else if (state->winner != WINNER_NONE)
        return -10000.0f;
    else
        return 0.0f; /* draw */
}
