"""
Random Evaluator - Only detects wins/losses, otherwise plays blind.

With no positional heuristics, this bot effectively picks randomly
among non-losing moves. Use it as a baseline.
"""

from src.connect4.board import GameState


class RandomEvaluator:
    """Baseline evaluator: only detects terminal states."""

    def evaluate(self, state: GameState, player: int) -> float:
        return 0.0

    def __repr__(self) -> str:
        return "Empty"


EVALUATOR = RandomEvaluator()
