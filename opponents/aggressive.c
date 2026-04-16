/*
 * Aggressive opponent: offense-biased window scoring.
 *
 * Uses custom asymmetric window weights:
 *   own 3-in-a-row  -> +120  (vs balanced +50)  -- hyper-offensive
 *   opp 3-in-a-row  -> -20   (vs balanced -50)  -- under-defends
 *   own 2-in-a-row  -> +10
 *   opp 2-in-a-row  -> -2
 *
 * Consequence: will race to build threats but will often let the opponent
 * complete a 4-in-a-row if blocking requires giving up an attack.
 *
 * Tests whether your evaluator can defend against a relentlessly
 * attacking bot without sacrificing its own counter-play.
 */

#include "../engine/board.h"

static float aggressive_score_window(int p_count, int o_count) {
    if (p_count > 0 && o_count > 0)
        return 0.0f; /* blocked */

    if (p_count == 4) return  500.0f;
    if (p_count == 3) return  120.0f; /* much higher than balanced */
    if (p_count == 2) return   10.0f;
    if (p_count == 1) return    1.0f;

    if (o_count == 4) return -500.0f;
    if (o_count == 3) return  -20.0f; /* much lower than balanced */
    if (o_count == 2) return   -2.0f;
    if (o_count == 1) return   -0.5f;

    return 0.0f;
}

float aggressive_evaluate(const GameState *state, int player) {
    int opponent = 1 - player;
    float score = 0.0f;
    int r, c, i;
    int p_count, o_count, cell;

    if (state->is_terminal) {
        if (state->winner == player)  return  10000.0f;
        if (state->winner != WINNER_NONE) return -10000.0f;
        return 0.0f;
    }

    /* Horizontal */
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r][c + i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            score += aggressive_score_window(p_count, o_count);
        }
    }

    /* Vertical */
    for (c = 0; c < COLS; c++) {
        for (r = 0; r <= ROWS - 4; r++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            score += aggressive_score_window(p_count, o_count);
        }
    }

    /* Diagonal: top-left to bottom-right */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 0; c <= COLS - 4; c++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c + i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            score += aggressive_score_window(p_count, o_count);
        }
    }

    /* Diagonal: top-right to bottom-left */
    for (r = 0; r <= ROWS - 4; r++) {
        for (c = 3; c < COLS; c++) {
            p_count = 0; o_count = 0;
            for (i = 0; i < 4; i++) {
                cell = state->board[r + i][c - i];
                if      (cell == player)   p_count++;
                else if (cell == opponent) o_count++;
            }
            score += aggressive_score_window(p_count, o_count);
        }
    }

    return score;
}
