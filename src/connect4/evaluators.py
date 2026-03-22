"""
Heuristic Evaluation Functions for Connect 4 with Removals.

⭐ THIS IS WHERE YOU DO YOUR WORK ⭐

This module contains evaluator classes that guide the minimax search.
A better evaluator leads to smarter AI, even with shallow search depth.

Evaluators combine heuristic functions (from heuristics.py) with weights
to create a position evaluation.

Your job is to:
1. Call heuristic functions to evaluate different aspects of a position
2. Combine them with weights to create your evaluator's score
3. Experiment with different weights to find what works best
"""

from .board import GameState
from .heuristics import (
    terminal_state_bonus,
    piece_count_advantage,
    center_control,
    threat_detection,
    winning_chances,
    removal_value,
)


class BasicEvaluator:
    """
    Minimal baseline evaluator: only detects terminal states.

    This is intentionally minimal—all heuristics are implemented as
    separate functions in heuristics.py that students can call.
    """

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate the position from the perspective of 'player'.

        Args:
            state: The game state to evaluate.
            player: The player we're evaluating for (0 or 1).

        Returns:
            A score. Higher = better for player, lower = worse for player.
        """
        # Only terminal state evaluation
        return terminal_state_bonus(state, player)


# ============================================================================
# STUDENT EVALUATOR EXAMPLE
# ============================================================================


class StudentEvaluator:
    """
    Example evaluator combining multiple heuristics.

    To create your own evaluator:
    1. Copy this class and rename it (e.g., MyEvaluator)
    2. Call heuristic functions to get scores
    3. Adjust the weights at the bottom to tune your AI
    4. Test and iterate!
    """

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate the position by combining multiple heuristics.

        Each heuristic is called as a function, then combined with weights.
        """
        # Call heuristics
        terminal_score = terminal_state_bonus(state, player)
        piece_score = piece_count_advantage(state, player)
        center_score = center_control(state, player)
        threat_score = threat_detection(state, player)
        winning_score = winning_chances(state, player)
        removal_score = removal_value(state, player)

        # Combine with weights (adjust these to tune behavior)
        score = (
            # terminal_score * 1 +
            # piece_score * 4 +
            # center_score * 1 +
            threat_score * 1 +
            winning_score * 5 
            # removal_score * 1
        )

        return score