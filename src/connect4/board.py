"""
Board state representation for Connect 4 with Removals.
"""

from dataclasses import dataclass, field
from typing import Optional
from copy import deepcopy


@dataclass
class GameState:
    """
    Represents the complete state of a Connect 4 game.

    Attributes:
        board: 6x7 board. Each cell is None (empty), 0 (player 0), or 1 (player 1).
               board[0] is the top row, board[5] is the bottom row.
        current_player: 0 or 1, whose turn it is.
        removals_remaining: List [player0_removals, player1_removals], max 1 each per game.
        is_terminal: True if game is over (someone won or board is full).
        winner: None if not terminal, 0 or 1 if someone won.
        move_history: List of moves made so far (for debugging/replay).
    """

    board: list = field(default_factory=lambda: [[None for _ in range(7)] for _ in range(6)])
    current_player: int = 0
    removals_remaining: list = field(default_factory=lambda: [1, 1])
    is_terminal: bool = False
    winner: Optional[int] = None
    move_history: list = field(default_factory=list)

    ROWS = 6
    COLS = 7

    def copy(self) -> "GameState":
        """Return a deep copy of the current game state."""
        return GameState(
            board=deepcopy(self.board),
            current_player=self.current_player,
            removals_remaining=self.removals_remaining.copy(),
            is_terminal=self.is_terminal,
            winner=self.winner,
            move_history=self.move_history.copy(),
        )

    def get_cell(self, row: int, col: int) -> Optional[int]:
        """Get the player at (row, col), or None if empty."""
        if 0 <= row < self.ROWS and 0 <= col < self.COLS:
            return self.board[row][col]
        return None

    def set_cell(self, row: int, col: int, player: Optional[int]) -> None:
        """Set the cell at (row, col) to the given player (or None for empty)."""
        if 0 <= row < self.ROWS and 0 <= col < self.COLS:
            self.board[row][col] = player

    def is_column_full(self, col: int) -> bool:
        """Check if a column is completely full."""
        for row in range(self.ROWS):
            if self.board[row][col] is None:
                return False
        return True

    def get_column_height(self, col: int) -> int:
        """
        Get the number of pieces in a column.
        Returns 0 if empty, up to 6 if full.
        """
        count = 0
        for row in range(self.ROWS):
            if self.board[row][col] is not None:
                count += 1
        return count

    def is_cell_occupied(self, row: int, col: int) -> bool:
        """Check if a cell has a piece."""
        return self.get_cell(row, col) is not None

    def count_pieces(self, player: int) -> int:
        """Count the number of pieces of a given player on the board."""
        count = 0
        for row in self.board:
            for cell in row:
                if cell == player:
                    count += 1
        return count

    def display(self) -> str:
        """Return a string representation of the board for printing."""
        lines = []
        for row_idx, row in enumerate(self.board):
            row_label = str(self.ROWS - 1 - row_idx)
            row_str = row_label + " |"
            for cell in row:
                if cell is None:
                    row_str += " .|"
                elif cell == 0:
                    row_str += " O|"
                else:
                    row_str += " X|"
            lines.append(row_str)
        lines.append("  +--+--+--+--+--+--+--+")
        lines.append("  | 0| 1| 2| 3| 4| 5| 6|")

        state_str = "\n".join(lines)
        state_str += f"\n\nCurrent player: {'O' if self.current_player == 0 else 'X'}"
        state_str += f"\nRemovals remaining: {self.removals_remaining}"

        if self.is_terminal:
            if self.winner is not None:
                state_str += f"\nGAME OVER: Player {'O' if self.winner == 0 else 'X'} wins!"
            else:
                state_str += "\nGAME OVER: Board is full."

        return state_str
