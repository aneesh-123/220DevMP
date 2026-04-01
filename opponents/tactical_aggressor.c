#include "test_opponent_utils.h"

float tactical_aggressor_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        1.2f, 0.0f, 0.5f, 0.8f,
        0.9f, 1.3f,
        2.2f, 7.5f, 17.0f,
        1.0f, 5.0f, 12.0f,
        0.9f, 1.7f, 4.5f, 1.5f
    };
    return evaluate_style(state, player, style);
}
