"""Benchmark evaluator that prefers build-up over urgent tactics."""

from ..board import GameState
from ..heuristics import (
    center_control_score,
    terminal_state_bonus,
    three_in_row_score,
    window_pattern_score,
)


class SetupBenchmark:
    """Reasonable-looking benchmark with no explicit immediate tactic term."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        score += 2.5 * center_control_score(state, player)
        score += 2.0 * window_pattern_score(state, player)
        score += 1.5 * three_in_row_score(state, player)
        return score

    def __repr__(self) -> str:
        return "Setup Benchmark"


EVALUATOR = SetupBenchmark()
