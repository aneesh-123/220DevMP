#include "test_opponent_utils.h"

float fortress_defender_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        1.0f, 0.3f, 0.5f, 0.7f,
        1.3f, 0.9f,
        1.4f, 4.0f, 8.0f,
        2.0f, 9.0f, 20.0f,
        1.0f, 1.2f, 2.5f, 3.0f
    };
    return evaluate_style(state, player, style);
}
