#include "test_opponent_utils.h"

float diagonal_specialist_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.9f, 0.1f, 0.4f, 0.7f,
        0.8f, 1.0f,
        1.8f, 6.0f, 13.0f,
        1.1f, 5.5f, 13.0f,
        0.4f, 3.5f, 3.5f, 1.8f
    };
    return evaluate_style(state, player, style);
}
