"""Benchmark evaluator that values generic open threes."""

from ..board import GameState
from ..heuristics import terminal_state_bonus, three_in_row_score, window_pattern_score


class ThreeBenchmark:
    """Threat-aware benchmark without playable-threat refinement."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        score += 1.5 * window_pattern_score(state, player)
        score += 1.5 * three_in_row_score(state, player)
        return score

    def __repr__(self) -> str:
        return "Three Benchmark"


EVALUATOR = ThreeBenchmark()
