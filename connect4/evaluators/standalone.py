"""Improved evaluator that recognizes immediate wins and forced blocks."""

from ..board import GameState
from ..heuristics import (
    center_control_score,
    immediate_win_and_block_score,
    terminal_state_bonus,
    three_in_row_score,
    window_pattern_score,
)


class ImmediateTacticsEvaluator:
    """Same baseline logic plus urgent win/block awareness."""

    def evaluate(self, state: GameState, player: int) -> float:
        score = 0.0
        score += terminal_state_bonus(state, player)
        score += 1.0 * immediate_win_and_block_score(state, player)
        return score

    def __repr__(self) -> str:
        return "Standalone"


EVALUATOR = ImmediateTacticsEvaluator()
