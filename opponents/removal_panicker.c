#include "test_opponent_utils.h"

float removal_panicker_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.7f, 0.1f, 0.5f, 0.6f,
        0.7f, 0.8f,
        1.5f, 4.5f, 9.0f,
        1.2f, 5.0f, 10.0f,
        0.6f, 0.9f, 1.8f, -6.0f
    };
    return evaluate_style(state, player, style);
}
