#include "test_opponent_utils.h"

float pure_positional_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        4.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f
    };
    return evaluate_style(state, player, style);
}
