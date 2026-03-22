"""Random Baseline - Only detects wins/losses, no positional awareness."""

from src.connect4.board import GameState
from src.connect4.heuristics import terminal_state_bonus


class RandomBaselineEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        return terminal_state_bonus(state, player)

    def __repr__(self) -> str:
        return "Random Baseline"


EVALUATOR = RandomBaselineEvaluator()
