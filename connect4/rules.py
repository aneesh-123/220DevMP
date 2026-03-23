"""
Game rules and mechanics for Connect 4 with Removals.

Includes:
- Legal move generation
- Move application (placement and removal with gravity)
- Win detection
- Terminal state detection
"""

from typing import List, Optional, Tuple
from .board import GameState
from .moves import PlacementMove, RemovalMove


def get_legal_moves(state: GameState) -> List:
    """
    Generate all legal moves for the current player.

    Returns a list of PlacementMove and/or RemovalMove objects.
    """
    moves = []

    # Placement moves: any non-full column
    for col in range(GameState.COLS):
        if not state.is_column_full(col):
            moves.append(PlacementMove(column=col))

    # Removal moves: any occupied cell (if player has removals left)
    if state.removals_remaining[state.current_player] > 0:
        for row in range(GameState.ROWS):
            for col in range(GameState.COLS):
                if state.is_cell_occupied(row, col):
                    moves.append(RemovalMove(row=row, col=col))

    return moves


def apply_move(state: GameState, move) -> GameState:
    """
    Apply a move to the state and return the updated state.

    Handles:
    - Placement: drop piece to lowest available row in column
    - Removal: remove piece and apply gravity
    - Win detection
    - Terminal state detection
    - Player switching
    """
    new_state = state.copy()
    new_state.move_history.append(move)

    if isinstance(move, PlacementMove):
        _apply_placement(new_state, move)
    elif isinstance(move, RemovalMove):
        _apply_removal(new_state, move)

    # Check for win
    if _check_win(new_state):
        new_state.winner = state.current_player
        new_state.is_terminal = True
    # Check for draw (board full)
    elif _is_board_full(new_state):
        new_state.is_terminal = True
        new_state.winner = None

    # Switch player if game is not terminal
    if not new_state.is_terminal:
        new_state.current_player = 1 - new_state.current_player

    return new_state


def _apply_placement(state: GameState, move: PlacementMove) -> None:
    """Apply a placement move: drop piece in column to lowest empty row."""
    col = move.column
    # Find the first empty row from the bottom
    for row in range(GameState.ROWS - 1, -1, -1):
        if state.get_cell(row, col) is None:
            state.set_cell(row, col, state.current_player)
            return
    # Should never reach here if move is legal
    raise ValueError(f"Column {col} is full but move was marked legal")


def _apply_removal(state: GameState, move: RemovalMove) -> None:
    """
    Apply a removal move: remove piece at (row, col) and apply gravity.

    Gravity: all pieces in the same column above the removed piece fall down by 1.
    """
    row, col = move.row, move.col

    # Remove the piece
    if state.get_cell(row, col) is None:
        raise ValueError(f"Cell ({row}, {col}) is empty but removal was marked legal")

    state.set_cell(row, col, None)

    # Apply gravity: shift pieces above the gap downward
    # Iterate from just above the gap (row-1) upward to the top (0)
    for source_row in range(row - 1, -1, -1):
        piece = state.get_cell(source_row, col)
        if piece is not None:
            state.set_cell(source_row + 1, col, piece)
            state.set_cell(source_row, col, None)

    # Decrement removal count for current player
    state.removals_remaining[state.current_player] -= 1


def _check_win(state: GameState) -> bool:
    """
    Check if the current player has won (4 in a row).

    Returns True if current player has created a 4-in-a-row.
    """
    player = state.current_player

    # Check horizontal
    for row in range(GameState.ROWS):
        for col in range(GameState.COLS - 3):
            if all(state.get_cell(row, col + i) == player for i in range(4)):
                return True

    # Check vertical
    for col in range(GameState.COLS):
        for row in range(GameState.ROWS - 3):
            if all(state.get_cell(row + i, col) == player for i in range(4)):
                return True

    # Check diagonal (top-left to bottom-right)
    for row in range(GameState.ROWS - 3):
        for col in range(GameState.COLS - 3):
            if all(state.get_cell(row + i, col + i) == player for i in range(4)):
                return True

    # Check diagonal (top-right to bottom-left)
    for row in range(GameState.ROWS - 3):
        for col in range(3, GameState.COLS):
            if all(state.get_cell(row + i, col - i) == player for i in range(4)):
                return True

    return False


def _is_board_full(state: GameState) -> bool:
    """Check if the board is completely full (draw condition)."""
    for col in range(GameState.COLS):
        if not state.is_column_full(col):
            return False
    return True


def has_legal_moves(state: GameState) -> bool:
    """Check if there are any legal moves remaining."""
    return len(get_legal_moves(state)) > 0
