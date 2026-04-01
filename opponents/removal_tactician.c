#include "test_opponent_utils.h"

float removal_tactician_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        1.1f, 0.2f, 0.5f, 0.8f,
        1.0f, 1.1f,
        1.9f, 6.5f, 15.0f,
        1.6f, 7.5f, 17.0f,
        1.0f, 1.5f, 3.5f, 4.0f
    };
    return evaluate_style(state, player, style);
}
