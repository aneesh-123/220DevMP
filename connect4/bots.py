"""
Bot implementations for Connect 4.

Only MinimaxBot is provided: uses minimax search with an evaluator.
"""

from .board import GameState
from .search import search


class MinimaxBot:
    """
    A bot that uses minimax search with alpha-beta pruning.

    The bot's strength is determined by the evaluation function quality.
    """

    def __init__(self, evaluator, depth: int = 5):
        """
        Initialize a minimax bot.

        Args:
            evaluator: An object with an evaluate(state, player) method.
                      This is where your heuristics go.
            depth: Search depth (default 5). Higher = slower but stronger.
        """
        self.evaluator = evaluator
        self.depth = depth

    def choose_move(self, state: GameState):
        """
        Choose the best move using minimax search.

        Args:
            state: The current game state.

        Returns:
            The best move found by minimax.
        """
        result = search(state, self.depth, self.evaluator)
        return result.best_move

    def __repr__(self) -> str:
        evaluator_name = self.evaluator.__class__.__name__
        return f"MinimaxBot(depth={self.depth}, evaluator={evaluator_name})"
