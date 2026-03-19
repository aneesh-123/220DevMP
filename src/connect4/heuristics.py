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
    Evaluator with THREAT DETECTION heuristic.

    This combines:
    1. Piece count (from BasicEvaluator)
    2. Center control (from BasicEvaluator)
    3. THREAT DETECTION (new): Heavily penalizes positions where opponent can win next move

    Threat detection is one of the most effective heuristics in Connect 4 because:
    - If opponent has a winning move, you MUST stop them (or you lose immediately)
    - By heavily penalizing threats, the bot prioritizes blocking
    """

    def evaluate(self, state: GameState, player: int) -> float:
        """
        Evaluate the position from the perspective of 'player'.

        Uses BasicEvaluator as foundation + adds threat detection penalty.
        """
        # Terminal states: immediate win/loss
        if state.is_terminal:
            if state.winner == player:
                return 10000  # Big win bonus
            elif state.winner is not None:
                return -10000  # Big loss penalty
            else:
                return 0  # Draw

        # Start with basic features
        my_pieces = state.count_pieces(player)
        opponent = 1 - player
        opponent_pieces = state.count_pieces(opponent)

        piece_diff = my_pieces - opponent_pieces

        # Center control bonus (from BasicEvaluator)
        center_bonus = 0
        for col in [2, 3, 4]:
            for row in range(state.ROWS):
                if state.get_cell(row, col) == player:
                    center_bonus += 1

        # NEW HEURISTIC: Threat detection
        # Count how many ways opponent can win next move
        opponent_threats = count_threats(state, opponent)
        threat_penalty = opponent_threats * 500  # Heavy penalty for each threat

        # Count player's own winning chances for next move (opportunity bonus)
        my_opportunities = count_winning_chances(state, player)
        opportunity_bonus = my_opportunities * 400  # Bonus for own winning moves

        # Combine all features
        score = (piece_diff * 10) + (center_bonus * 1) + opportunity_bonus - threat_penalty

        return score


# ============================================================================
# HELPER FUNCTIONS (You may implement these to support your heuristics)
# ============================================================================

def count_threats(state: GameState, opponent: int) -> int:
    """
    Count how many ways 'opponent' can win in the next move.

    High threat count = opponent is dangerous, so this position is bad for 'player'.

    This works by checking each empty cell: if the opponent could place a piece there
    and create 4-in-a-row, it counts as a threat.
    """
    threat_count = 0

    # Check each cell on the board
    for row in range(state.ROWS):
        for col in range(state.COLS):
            # Only check empty cells
            if state.get_cell(row, col) is None:
                # Temporarily place opponent's piece
                state.set_cell(row, col, opponent)

                # Check if this creates a 4-in-a-row for opponent
                if _check_win_for_player(state, opponent):
                    threat_count += 1

                # Remove the temporary piece
                state.set_cell(row, col, None)

    return threat_count


def _check_win_for_player(state: GameState, player: int) -> bool:
    """
    Check if 'player' has 4-in-a-row on the board.

    Unlike rules._check_win(), this doesn't require state.current_player to match.
    """
    # Check horizontal
    for row in range(state.ROWS):
        for col in range(state.COLS - 3):
            if all(state.get_cell(row, col + i) == player for i in range(4)):
                return True

    # Check vertical
    for col in range(state.COLS):
        for row in range(state.ROWS - 3):
            if all(state.get_cell(row + i, col) == player for i in range(4)):
                return True

    # Check diagonal (top-left to bottom-right)
    for row in range(state.ROWS - 3):
        for col in range(state.COLS - 3):
            if all(state.get_cell(row + i, col + i) == player for i in range(4)):
                return True

    # Check diagonal (top-right to bottom-left)
    for row in range(state.ROWS - 3):
        for col in range(3, state.COLS):
            if all(state.get_cell(row + i, col - i) == player for i in range(4)):
                return True

    return False


def count_winning_chances(state: GameState, player: int) -> int:
    """
    Count how many ways 'player' can win in the next move.

    High count = player is in a winning position (has multiple winning moves).

    This works by checking each empty cell: if the player could place a piece there
    and create 4-in-a-row, it counts as a winning chance.
    """
    chance_count = 0

    # Check each cell on the board
    for row in range(state.ROWS):
        for col in range(state.COLS):
            # Only check empty cells
            if state.get_cell(row, col) is None:
                # Temporarily place player's piece
                state.set_cell(row, col, player)

                # Check if this creates a 4-in-a-row for player
                if _check_win_for_player(state, player):
                    chance_count += 1

                # Remove the temporary piece
                state.set_cell(row, col, None)

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
