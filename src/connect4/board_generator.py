"""
Utility to generate valid random board states for experiments.

Run directly: python -m src.connect4.board_generator
"""

import json
import random
from pathlib import Path


def generate_random_board(num_pieces: int, seed: int = None, removals=None):
    """
    Generate a valid random board by simulating alternating drops.

    Returns a dict ready to write as JSON, or None if no valid board
    could be generated (e.g., 4-in-a-row appeared before target piece count).
    """
    if seed is not None:
        random.seed(seed)
    if removals is None:
        removals = [1, 1]

    board = [[None for _ in range(7)] for _ in range(6)]

    for i in range(num_pieces):
        player = i % 2
        # Pick a random non-full column
        open_cols = [c for c in range(7) if board[0][c] is None]
        if not open_cols:
            return None
        col = random.choice(open_cols)

        # Drop piece (find lowest empty row)
        for row in range(5, -1, -1):
            if board[row][col] is None:
                board[row][col] = player
                break

        # Check for 4-in-a-row (invalid for a starting position)
        if _has_four_in_a_row(board):
            return None

    name_map = {
        4: "Early Game",
        8: "Mid Game Open",
        12: "Mid Game Contested",
        16: "Late Game",
        10: "Asymmetric",
    }
    name = name_map.get(num_pieces, f"{num_pieces} Pieces")

    return {
        "name": f"{name} ({num_pieces} pieces)",
        "current_player": 0,
        "removals_remaining": removals,
        "board": board,
    }


def _has_four_in_a_row(board):
    """Check if any player has 4 in a row."""
    for player in [0, 1]:
        for row in range(6):
            for col in range(4):
                if all(board[row][col + i] == player for i in range(4)):
                    return True
        for col in range(7):
            for row in range(3):
                if all(board[row + i][col] == player for i in range(4)):
                    return True
        for row in range(3):
            for col in range(4):
                if all(board[row + i][col + i] == player for i in range(4)):
                    return True
        for row in range(3):
            for col in range(3, 7):
                if all(board[row + i][col - i] == player for i in range(4)):
                    return True
    return False


def generate_and_save():
    """Generate board states and save to board_states/ folder."""
    output_dir = Path(__file__).parent / "board_states"

    configs = [
        (4, "early_game"),
        (8, "mid_game_open"),
        (12, "mid_game_contested"),
        (16, "late_game"),
        (10, "asymmetric"),
    ]

    for num_pieces, filename in configs:
        # Try seeds until we get a valid board
        for seed in range(1000):
            removals = [0, 1] if filename == "asymmetric" else [1, 1]
            result = generate_random_board(num_pieces, seed=seed, removals=removals)
            if result is not None:
                filepath = output_dir / f"{filename}.json"
                with open(filepath, "w") as f:
                    json.dump(result, f, indent=2)
                print(f"Generated {filepath.name}: {result['name']}")
                break
        else:
            print(f"Failed to generate valid board for {filename}")


if __name__ == "__main__":
    generate_and_save()
