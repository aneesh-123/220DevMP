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
    threat_detection,
    piece_count_advantage,
    removal_value,
)


class CenterWinEvaluator:
    """Offensive evaluator that seeks wins through center control."""

    def evaluate(self, state: GameState, player: int) -> float:
        terminal_score = terminal_state_bonus(state, player)
        threat_score = threat_detection(state, player)
        center_score = center_control(state, player)
        winning_score = winning_chances(state, player)
        piece_count_score = piece_count_advantage(state, player)
        removal_score = removal_value(state, player)

        score = terminal_score + threat_score * 5 + center_score * 5 + winning_score * 10 + piece_count_score * 2 + removal_score * 3

        return score

    def __repr__(self) -> str:
        return "Master"


EVALUATOR = CenterWinEvaluator()
