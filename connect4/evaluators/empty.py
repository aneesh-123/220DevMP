"""
Starter evaluator for student heuristic design.

Add reusable heuristic helper functions in connect4/heuristics.py,
then combine them here to build a stronger evaluator.
"""

from ..board import GameState
from ..heuristics import terminal_state_bonus


class RandomEvaluator:
    """Baseline evaluator built from heuristic helper functions."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        return score

    def __repr__(self) -> str:
        return "Empty"


EVALUATOR = RandomEvaluator()
