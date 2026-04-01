#include "test_opponent_utils.h"

float endgame_closer_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.7f, 0.1f, 0.8f, 0.6f,
        1.0f, 0.9f,
        1.5f, 9.0f, 25.0f,
        1.4f, 8.0f, 22.0f,
        1.2f, 1.6f, 3.0f, 2.5f
    };
    return evaluate_style(state, player, style);
}
