"""
Center Winner - Aggressively seeks winning positions through the center.

Heavily weights winning chances and center control to create an
offensive bot that builds toward wins from strong board positions.
"""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    center_control,
    winning_chances,
)


class CenterWinEvaluator:
    """Offensive evaluator that seeks wins through center control."""

    def evaluate(self, state: GameState, player: int) -> float:
        terminal_score = terminal_state_bonus(state, player)
        center_score = center_control(state, player)
        winning_score = winning_chances(state, player)

        score = terminal_score + center_score * 3 + winning_score * 10

        return score

    def __repr__(self) -> str:
        return "Center Winner"


EVALUATOR = CenterWinEvaluator()
