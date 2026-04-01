#include "test_opponent_utils.h"

float threat_reactor_evaluate(const GameState *state, int player) {
    OpponentStyle style = {
        0.3f, 0.0f, 0.2f, 0.3f,
        0.4f, 0.2f,
        0.3f, 1.0f, 2.0f,
        2.5f, 9.0f, 18.0f,
        0.3f, 0.5f, 0.5f, 1.5f
    };
    return evaluate_style(state, player, style);
}
