"""
Bot B - Balanced Strategy

This bot uses multiple heuristics to evaluate positions: terminal states,
piece count, center control, and removal value. This creates a more balanced
approach compared to Bot A's threat-focused strategy.
"""

from src.connect4.board import GameState
from src.connect4.heuristics import (
    terminal_state_bonus,
    piece_count_advantage,
    center_control,
    removal_value,
)
from src.connect4.bots import MinimaxBot


class BotBEvaluator:
    """Evaluator using balanced heuristics for general board control."""

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate position using multiple balanced heuristics.

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

        # Use multiple heuristics for balanced evaluation
        piece_score = piece_count_advantage(state, player)
        center_score = center_control(state, player)
        removal_score = removal_value(state, player)

        # Combine heuristics with equal weight
        score = piece_score * 1 + center_score * 1 + removal_score * 1
        return score

    def __repr__(self) -> str:
        return "Bot B (Balanced)"


# Create the bot instance
BOT = MinimaxBot(evaluator=BotBEvaluator(), depth=4)
