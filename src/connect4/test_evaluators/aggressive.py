"""Aggressive - Seeks wins through center control and winning chances."""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    center_control,
    winning_chances,
)


class AggressiveEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        return (
            terminal_state_bonus(state, player) +
            center_control(state, player) * 3 +
            winning_chances(state, player) * 10
        )

    def __repr__(self) -> str:
        return "Aggressive"


EVALUATOR = AggressiveEvaluator()
