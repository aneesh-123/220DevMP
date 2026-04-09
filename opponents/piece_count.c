/*
 * Piece-count opponent: terminal bonus + raw material advantage.
 *
 * Scores the difference between own pieces and opponent pieces on the board.
 * Knows to win when possible and avoid losing, and mildly prefers accumulating
 * material. Has no spatial awareness — treats all cells equally.
 *
 * Should consistently beat terminal_only but lose to any bot that considers
 * board position or threats.
 */

#include "../engine/board.h"

float piece_count_evaluate(const GameState *state, int player) {
    int opponent = 1 - player;

    if (state->is_terminal) {
        if (state->winner == player)
            return 10000.0f;
        else if (state->winner != WINNER_NONE)
            return -10000.0f;
        else
            return 0.0f;
    }

    return (float)(gamestate_count_pieces(state, player)
                 - gamestate_count_pieces(state, opponent));
}
