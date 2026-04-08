#include "../heuristics.h"

float final_evaluator(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);

    return score;
}