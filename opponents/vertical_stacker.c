#include "test_opponent_utils.h"

float vertical_stacker_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.6f, 0.0f, 0.4f, 0.4f,
        1.0f, 0.6f,
        1.0f, 4.0f, 8.0f,
        0.8f, 4.0f, 8.0f,
        4.5f, 0.3f, 1.0f, 0.5f
    };
    return evaluate_style(state, player, style);
}
