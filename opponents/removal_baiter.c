#include "test_opponent_utils.h"

float removal_baiter_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        1.0f, 0.0f, 0.6f, 0.9f,
        1.2f, 1.3f,
        2.3f, 7.0f, 18.0f,
        1.3f, 6.5f, 16.0f,
        0.9f, 2.0f, 5.0f, 7.0f
    };
    return evaluate_style(state, player, style);
}
