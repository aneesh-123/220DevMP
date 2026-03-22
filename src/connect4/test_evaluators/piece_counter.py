"""Piece Counter - Values having more pieces and removal flexibility."""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    piece_count_advantage,
    removal_value,
)


class PieceCounterEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        return (
            terminal_state_bonus(state, player) +
            piece_count_advantage(state, player) * 2 +
            removal_value(state, player) * 1
        )

    def __repr__(self) -> str:
        return "Piece Counter"


EVALUATOR = PieceCounterEvaluator()
