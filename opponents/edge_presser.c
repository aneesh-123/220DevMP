#include "test_opponent_utils.h"

float edge_presser_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        -1.5f, 4.0f, 0.2f, 0.0f,
        0.3f, 0.3f,
        0.5f, 1.5f, 3.0f,
        0.0f, 0.5f, 1.0f,
        0.5f, 0.3f, 0.3f, 0.0f
    };
    return evaluate_style(state, player, style);
}
