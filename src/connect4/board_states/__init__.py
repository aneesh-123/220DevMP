"""
Board state loader for Connect 4 experiments.

Loads pre-made board configurations from JSON files in this folder.
Each board state provides a unique starting position for experiment games.
"""

import json
from pathlib import Path
from typing import List, Tuple
from ..board import GameState


def load_board_states() -> List[Tuple[str, GameState]]:
    """
    Load all board states from JSON files in this folder.

    Returns:
        List of (name, GameState) tuples.
    """
    states = []
    state_dir = Path(__file__).parent

    for state_file in sorted(state_dir.glob("*.json")):
        try:
            with open(state_file) as f:
                data = json.load(f)

            board = data["board"]

            if not _validate_gravity(board):
                print(f"Warning: Skipping {state_file.name} (invalid gravity)")
                continue

            state = GameState(
                board=board,
                current_player=data.get("current_player", 0),
                removals_remaining=data.get("removals_remaining", [1, 1]),
            )

            name = data.get("name", state_file.stem)
            states.append((name, state))

        except Exception as e:
            print(f"Warning: Could not load board state from {state_file.name}: {e}")

    return states


def _validate_gravity(board) -> bool:
    """Check that no piece is floating above an empty cell."""
    for col in range(7):
        found_piece = False
        for row in range(6):  # top to bottom
            if board[row][col] is not None:
                found_piece = True
            elif found_piece:
                # Empty cell below a piece = floating
                return False
    return True
