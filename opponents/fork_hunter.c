#include "test_opponent_utils.h"

float fork_hunter_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.7f, 0.0f, 0.3f, 0.5f,
        0.6f, 0.9f,
        1.8f, 5.5f, 11.0f,
        0.5f, 2.5f, 5.0f,
        0.5f, 1.5f, 6.0f, 0.8f
    };
    return evaluate_style(state, player, style);
}
