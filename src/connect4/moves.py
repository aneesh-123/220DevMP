"""
Move representations for Connect 4 with Removals.

Two types of moves:
- PlacementMove: Drop a piece in a column
- RemovalMove: Remove a piece from the board
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class PlacementMove:
    """
    A placement move: drop a piece into a column.

    Attributes:
        column: The column (0-6) where the piece will be placed.
    """
    column: int

    def __str__(self) -> str:
        return f"Place in column {self.column}"

    def __repr__(self) -> str:
        return f"PlacementMove({self.column})"


@dataclass(frozen=True)
class RemovalMove:
    """
    A removal move: remove a piece from the board.

    After removal, gravity applies (pieces above fall downward).

    Attributes:
        row: The row (0-5, 0 is top) of the piece to remove.
        col: The column (0-6) of the piece to remove.
    """
    row: int
    col: int

    def __str__(self) -> str:
        return f"Remove from ({self.row}, {self.col})"

    def __repr__(self) -> str:
        return f"RemovalMove({self.row}, {self.col})"
