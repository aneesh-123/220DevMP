"""Defensive - Focuses on blocking opponent threats and holding the center."""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    threat_detection,
    center_control,
)


class DefensiveEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        return (
            terminal_state_bonus(state, player) +
            threat_detection(state, player) * 5 +
            center_control(state, player) * 1
        )

    def __repr__(self) -> str:
        return "Defensive"


EVALUATOR = DefensiveEvaluator()
