"""
Heuristic Functions for Connect 4 with Removals.

This module contains all heuristic functions that evaluators can call
to assess different aspects of a game position.

Each heuristic returns a numerical score that can be weighted and combined
to create an overall position evaluation.
"""

from .board import GameState


# ============================================================================
# HEURISTIC FUNCTIONS: Call these to evaluate different aspects of position
# ============================================================================


def terminal_state_bonus(state: GameState, player: int) -> float:
    """
    Detect terminal states and return win/loss/draw bonus.

    If the game is over, returns a definitive score:
    - 10000 if player won
    - -10000 if player lost
    - 0 if draw

    If the game is not over, returns None (non-terminal).

    Args:
        state: The game state.
        player: The player to evaluate for.

    Returns:
        Terminal state score, or None if game is not over.
    """
    if not state.is_terminal:
        return None

    if state.winner == player:
        return 10000
    elif state.winner is not None:
        return -10000
    else:
        return 0


def piece_count_advantage(state: GameState, player: int) -> float:
    """
    Evaluate piece count difference.

    More pieces = more control and potential winning combinations.

    Args:
        state: The game state.
        player: The player to evaluate for.

    Returns:
        The difference in piece counts (player's pieces - opponent's pieces).
    """
    my_pieces = state.count_pieces(player)
    opponent_pieces = state.count_pieces(1 - player)
    return my_pieces - opponent_pieces


def center_control(state: GameState, player: int) -> float:
    """
    Evaluate control of center columns.

    Center columns (2, 3, 4) offer more winning opportunities because
    pieces placed there can form more potential 4-in-a-rows.

    Args:
        state: The game state.
        player: The player to evaluate for.

    Returns:
        Count of player's pieces in center columns.
    """
    center_count = 0
    for col in [2, 3, 4]:
        for row in range(state.ROWS):
            if state.get_cell(row, col) == player:
                center_count += 1
    return center_count


def threat_detection(state: GameState, player: int) -> float:
    """
    Penalize positions where opponent can win next move.

    If opponent has a winning move, you MUST block them or lose immediately.
    This heuristic prioritizes defensive play.

    Args:
        state: The game state.
        player: The player to evaluate for.

    Returns:
        Negative score proportional to opponent's winning threats.
    """
    opponent = 1 - player
    opponent_threats = count_threats(state, opponent)
    return -opponent_threats * 500


def winning_chances(state: GameState, player: int) -> float:
    """
    Bonus for positions where player can win next move.

    Multiple winning moves = player is in a strong position.

    Args:
        state: The game state.
        player: The player to evaluate for.

    Returns:
        Positive score proportional to player's winning opportunities.
    """
    my_opportunities = count_winning_chances(state, player)
    return my_opportunities * 400


def removal_value(state: GameState, player: int) -> float:
    """
    Evaluate the remaining removal advantage.

    More removals left = more flexibility and strategic options.

    Args:
        state: The game state.
        player: The player to evaluate for.

    Returns:
        Score based on removal advantage.
    """
    removals_remaining = state.removals_remaining[player]
    opponent_removals = state.removals_remaining[1 - player]
    return (removals_remaining - opponent_removals) * 50


# ============================================================================
# HELPER FUNCTIONS: Used by heuristics above
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
