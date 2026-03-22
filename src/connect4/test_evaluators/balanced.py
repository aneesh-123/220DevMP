"""Balanced - Uses all heuristics with tuned weights. Hardest opponent."""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    piece_count_advantage,
    center_control,
    threat_detection,
    winning_chances,
    removal_value,
)


class BalancedEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        return (
            terminal_state_bonus(state, player) +
            piece_count_advantage(state, player) * 1 +
            center_control(state, player) * 2 +
            threat_detection(state, player) * 3 +
            winning_chances(state, player) * 4 +
            removal_value(state, player) * 1
        )

    def __repr__(self) -> str:
        return "Balanced"


EVALUATOR = BalancedEvaluator()
