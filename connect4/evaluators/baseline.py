"""Baseline evaluator using generic three-in-a-row scoring."""

from ..board import GameState
from ..heuristics import (
    center_control_score,
    terminal_state_bonus,
    three_in_row_score,
    window_pattern_score,
)


class BaselineThreesEvaluator:
    """Scores open threes without checking whether they are playable now."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        score += 3.0 * center_control_score(state, player)
        score += 2.0 * window_pattern_score(state, player)
        score += 1.5 * three_in_row_score(state, player)
        return score

    def __repr__(self) -> str:
        return "Baseline"


EVALUATOR = BaselineThreesEvaluator()
