"""Benchmark evaluator based on generic window scoring."""

from ..board import GameState
from ..heuristics import terminal_state_bonus, window_pattern_score


class WindowBenchmark:
    """Reasonable opponent that values strong-looking windows."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        score += 2.0 * window_pattern_score(state, player)
        return score

    def __repr__(self) -> str:
        return "Window Benchmark"


EVALUATOR = WindowBenchmark()
