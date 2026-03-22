"""
Threat Blocker - Prioritizes blocking opponent threats.

Heavily weights threat detection so the bot focuses on defense:
if the opponent has a winning move, block it at all costs.
"""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    threat_detection,
)


class ThreatBlockerEvaluator:
    """Defensive evaluator that prioritizes blocking opponent threats."""

    def evaluate(self, state: GameState, player: int) -> float:
        terminal_score = terminal_state_bonus(state, player)
        threat_score = threat_detection(state, player)

        score = terminal_score + threat_score * 5

        return score

    def __repr__(self) -> str:
        return "Threat Blocker"


EVALUATOR = ThreatBlockerEvaluator()
