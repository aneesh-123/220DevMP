"""
Bot A - Threat Detection Strategy

This bot uses threat detection and winning chances as its primary heuristics.
It aggressively blocks opponent threats and capitalizes on winning opportunities.
"""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    threat_detection,
    winning_chances,
)
from src.connect4.bots import MinimaxBot


class BotAEvaluator:
    """Evaluator focused on threat detection and winning chances."""

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate position by detecting threats and scoring winning opportunities.

        Args:
            state: The game state to evaluate.
            player: The player we're evaluating for (0 or 1).

        Returns:
            A score where higher = better for player.
        """
        # Detect terminal states first (these are most important)
        terminal_score = terminal_state_bonus(state, player)
        if terminal_score != 0:
            return terminal_score

        # Focus on threat detection and winning chances
        threat_score = threat_detection(state, player)
        winning_score = winning_chances(state, player)

        # Combine heuristics
        score = threat_score * 1 + winning_score * 5
        return score

    def __repr__(self) -> str:
        return "Bot A (Threat Detection)"


# Create the bot instance
BOT = MinimaxBot(evaluator=BotAEvaluator(), depth=4)
