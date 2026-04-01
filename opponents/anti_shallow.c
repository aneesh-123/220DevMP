#include "test_opponent_utils.h"

float anti_shallow_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        1.0f, 0.2f, 0.4f, 0.7f,
        1.0f, 1.1f,
        2.0f, 6.5f, 14.0f,
        1.8f, 8.5f, 19.0f,
        0.8f, 1.8f, 5.5f, 2.5f
    };
    return evaluate_style(state, player, style);
}
