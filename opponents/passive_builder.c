#include "test_opponent_utils.h"

float passive_builder_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.5f, 0.0f, 0.3f, 0.2f,
        0.4f, 0.6f,
        2.5f, 7.0f, 12.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.5f, 0.0f
    };
    return evaluate_style(state, player, style);
}
