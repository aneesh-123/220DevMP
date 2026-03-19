"""
Heuristic Evaluation Functions for Connect 4.

⭐ THIS IS WHERE YOU DO YOUR WORK ⭐

This module contains evaluation functions that guide the minimax search.
A better evaluator leads to smarter AI, even with shallow search depth.

The BasicEvaluator below is intentionally weak—it's just enough to make
the code run. Replace or extend it with your own heuristics.

Your job is to:
1. Define features that matter in Connect 4 (territory, threats, removals, etc.)
2. Score those features
3. Combine them into an evaluation function
4. Test different configurations and see what works best
"""

from .board import GameState


class BasicEvaluator:
    """
    A minimal baseline evaluator to make the code runnable.

    This is intentionally weak and should be replaced with your own heuristics.
    """

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate the position from the perspective of 'player'.

        Args:
            state: The game state to evaluate.
            player: The player we're evaluating for (0 or 1).

        Returns:
            A score. Higher = better for player, lower = worse for player.
            Typically use large values (±1000) for wins/losses.
        """
        # Terminal states: immediate win/loss
        if state.is_terminal:
            if state.winner == player:
                return 10000  # Big win bonus
            elif state.winner is not None:
                return -10000  # Big loss penalty
            else:
                return 0  # Draw

        # Baseline: piece count advantage (minimal heuristic)
        my_pieces = state.count_pieces(player)
        opponent = 1 - player
        opponent_pieces = state.count_pieces(opponent)

        piece_diff = my_pieces - opponent_pieces

        # Tiny preference for center (pieces in columns 2-4 vs edges)
        center_bonus = self._center_control(state, player)

        return piece_diff * 10 + center_bonus

    def _center_control(self, state: GameState, player: int) -> float:
        """
        Tiny bonus for having pieces in center columns (2, 3, 4).

        Not a strong heuristic, just a placeholder.
        """
        bonus = 0
        for col in [2, 3, 4]:
            for row in range(state.ROWS):
                if state.get_cell(row, col) == player:
                    bonus += 1
        return bonus * 1  # Tiny weight


# ============================================================================
# STUDENT HEURISTICS: Add your evaluators below
# ============================================================================

class StudentEvaluator:
    """
    YOUR EVALUATOR HERE.

    Replace or extend BasicEvaluator with features you think are important.

    Suggested features to explore:
    - Threat detection: Opponent can win in 1 move? High penalty.
    - Opportunity: You can win in 1 move? High bonus.
    - Territory: Control of center, key columns, key rows.
    - Clustering: Pieces grouped together (potential for 4-in-a-row).
    - Removal count: How many removals left for each player? Strategic value?
    - Fragmentation: Opponent's pieces spread thin = good for you.
    - Front row advantage: Pieces closer to playable columns might be stronger.

    TODO: Implement your heuristic features below.
    """

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate the position from the perspective of 'player'.

        TODO: Implement your heuristic logic here.
        """
        # PLACEHOLDER: Just copy the baseline for now
        evaluator = BasicEvaluator()
        return evaluator.evaluate(state, player)


# ============================================================================
# HELPER FUNCTIONS (You may implement these to support your heuristics)
# ============================================================================

def count_threats(state: GameState, player: int, opponent: int) -> int:
    """
    Count how many ways 'opponent' can win in the next move.

    High threat count = opponent is dangerous, so this position is bad for 'player'.

    TODO: Implement if you want threat-based heuristics.
    """
    threat_count = 0
    # TODO: For each empty cell, check if filling it with opponent's piece creates 4-in-a-row
    return threat_count


def count_winning_chances(state: GameState, player: int) -> int:
    """
    Count how many ways 'player' can win in the next move.

    High count = player is in a winning position.

    TODO: Implement if you want opportunity-based heuristics.
    """
    chance_count = 0
    # TODO: For each empty cell, check if filling it with player's piece creates 4-in-a-row
    return chance_count


def evaluate_piece_clustering(state: GameState, player: int) -> float:
    """
    Evaluate how well a player's pieces are clustered.

    Pieces that are close together have more potential to form 4-in-a-row.

    TODO: Implement if you want clustering heuristics.

    Returns:
        A score reflecting how well pieces are grouped.
    """
    # TODO: Calculate clustering score (e.g., average distance between ally pieces)
    return 0.0


def evaluate_column_strength(state: GameState, player: int) -> float:
    """
    Evaluate the column-by-column strength for a player.

    Some columns might be "owned" (many pieces, few opponent pieces).

    TODO: Implement if you want position-based heuristics.

    Returns:
        A score reflecting column control.
    """
    # TODO: Score each column based on piece composition
    return 0.0


def evaluate_removal_value(state: GameState, player: int) -> float:
    """
    Evaluate the value of removals still available.

    More removals left = more flexibility = potentially better position.

    TODO: Implement if you want removal-aware heuristics.

    Returns:
        A score reflecting removal availability.
    """
    # TODO: Factor in remaining removals for both players
    removals_remaining = state.removals_remaining[player]
    opponent_removals = state.removals_remaining[1 - player]
    return (removals_remaining - opponent_removals) * 50


# ============================================================================
# EXPERIMENTATION NOTES
# ============================================================================

"""
When you're ready to experiment:

1. Create a new evaluator class (e.g., MyHeuristic1, MyHeuristic2, etc.)
2. Implement evaluate() to score positions based on your ideas
3. Use the experiment.py module to run many games and compare outcomes

Example:
    from connect4.heuristics import MyHeuristic1, MyHeuristic2
    from connect4.bots import MinimaxBot
    from connect4.experiment import Experiment

    bot1 = MinimaxBot(evaluator=MyHeuristic1(), depth=5)
    bot2 = MinimaxBot(evaluator=MyHeuristic2(), depth=5)

    experiment = Experiment(bot1=bot1, bot2=bot2, num_games=20, seed=42)
    results = experiment.run()
    print(results)

Track which features help and which hurt. Iterate!
"""
