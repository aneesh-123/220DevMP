#include "test_opponent_utils.h"

float trap_setter_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.8f, 0.0f, 0.2f, 0.5f,
        0.7f, 1.2f,
        2.8f, 7.5f, 16.0f,
        0.3f, 1.5f, 4.0f,
        0.2f, 2.5f, 10.0f, 1.5f
    };
    return evaluate_style(state, player, style);
}
