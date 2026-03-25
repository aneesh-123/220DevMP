"""Benchmark evaluator focused on center control."""

from ..board import GameState
from ..heuristics import center_control_score, terminal_state_bonus


class CenterBenchmark:
    """Stable positional benchmark with no special threat logic."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        score += 3.0 * center_control_score(state, player)
        return score

    def __repr__(self) -> str:
        return "Center Benchmark"


EVALUATOR = CenterBenchmark()
