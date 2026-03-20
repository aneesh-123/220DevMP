"""
Tests for win detection.
"""

import pytest
from connect4.board import GameState
from connect4.moves import PlacementMove, RemovalMove
from connect4.rules import apply_move, _check_win


class TestWinDetectionHorizontal:
    """Test horizontal 4-in-a-row detection."""

    def test_horizontal_win(self):
        """Four in a row horizontally should be detected."""
        state = GameState()
        # Place 4 in a row at bottom
        for col in range(4):
            state.set_cell(5, col, 0)

        state.current_player = 0
        assert _check_win(state)

    def test_horizontal_win_different_row(self):
        """Win should work on any row."""
        state = GameState()
        for col in range(4):
            state.set_cell(0, col, 1)

        state.current_player = 1
        assert _check_win(state)

    def test_horizontal_incomplete(self):
        """Three in a row should not be a win."""
        state = GameState()
        for col in range(3):
            state.set_cell(5, col, 0)

        state.current_player = 0
        assert not _check_win(state)

    def test_horizontal_with_gap(self):
        """Pieces with a gap should not be a win."""
        state = GameState()
        state.set_cell(5, 0, 0)
        state.set_cell(5, 1, 0)
        state.set_cell(5, 2, 1)  # Gap with opponent
        state.set_cell(5, 3, 0)

        state.current_player = 0
        assert not _check_win(state)

    def test_horizontal_different_columns(self):
        """Test 4-in-a-row in middle of row."""
        state = GameState()
        for col in range(2, 6):
            state.set_cell(5, col, 1)

        state.current_player = 1
        assert _check_win(state)


class TestWinDetectionVertical:
    """Test vertical 4-in-a-row detection."""

    def test_vertical_win(self):
        """Four in a column should be detected."""
        state = GameState()
        for row in range(2, 6):
            state.set_cell(row, 3, 0)

        state.current_player = 0
        assert _check_win(state)

    def test_vertical_incomplete(self):
        """Three in a column should not be a win."""
        state = GameState()
        for row in range(3, 6):
            state.set_cell(row, 3, 1)

        state.current_player = 1
        assert not _check_win(state)

    def test_vertical_with_gap(self):
        """Pieces with a gap should not be a win."""
        state = GameState()
        state.set_cell(2, 3, 0)
        state.set_cell(3, 3, 0)
        state.set_cell(4, 3, 1)  # Gap
        state.set_cell(5, 3, 0)

        state.current_player = 0
        assert not _check_win(state)


class TestWinDetectionDiagonal:
    """Test diagonal 4-in-a-row detection."""

    def test_diagonal_down_right(self):
        """Diagonal down-right should be detected."""
        state = GameState()
        # Pieces at (5,0), (4,1), (3,2), (2,3)
        for i in range(4):
            state.set_cell(5 - i, 0 + i, 0)

        state.current_player = 0
        assert _check_win(state)

    def test_diagonal_up_right(self):
        """Diagonal up-right should be detected."""
        state = GameState()
        # Pieces at (5,0), (4,1), (3,2), (2,3)
        for i in range(4):
            state.set_cell(2 + i, 3 - i, 1)

        state.current_player = 1
        assert _check_win(state)

    def test_diagonal_incomplete(self):
        """Three on diagonal should not be a win."""
        state = GameState()
        for i in range(3):
            state.set_cell(5 - i, 0 + i, 0)

        state.current_player = 0
        assert not _check_win(state)

    def test_diagonal_with_gap(self):
        """Diagonal with a gap should not be a win."""
        state = GameState()
        state.set_cell(5, 0, 0)
        state.set_cell(4, 1, 0)
        state.set_cell(3, 2, 1)  # Gap
        state.set_cell(2, 3, 0)

        state.current_player = 0
        assert not _check_win(state)


class TestWinDetectionWithMoves:
    """Test win detection through actual move application."""

    def test_win_by_placement(self):
        """Win should be detected immediately after placement."""
        state = GameState()
        state.set_cell(5, 0, 0)
        state.set_cell(5, 1, 0)
        state.set_cell(5, 2, 0)

        state = apply_move(state, PlacementMove(column=3))

        assert state.is_terminal
        assert state.winner == 0

    def test_win_by_removal(self):
        """Removal followed by placement can create a win."""
        state = GameState()
        # Setup: three in a row with opponent piece blocking
        state.set_cell(5, 0, 0)
        state.set_cell(5, 1, 0)
        state.set_cell(5, 2, 0)
        state.set_cell(5, 3, 1)

        # P0 removes the blocking piece
        state.current_player = 0
        state = apply_move(state, RemovalMove(row=5, col=3))

        # After removal, it's P1's turn and P0 doesn't have 4-in-a-row yet
        # (the gap is still there)
        assert not state.is_terminal
        assert state.current_player == 1

    def test_no_win_on_opponent_placement(self):
        """Should only check win for the current player."""
        state = GameState()
        # P0 places 4 in a row
        for col in range(4):
            state.set_cell(5, col, 0)

        state.current_player = 1
        # P1 places a piece (not checking P0's existing win)
        state = apply_move(state, PlacementMove(column=4))

        # P1 should have just taken a turn, current player is now P0
        # But the win should have been detected at P1's move application? No.
        # Win is checked from the perspective of the player who just moved.
        # P1 just moved, so we check if P1 created 4-in-a-row.
        assert not state.is_terminal  # P1's move didn't create a win


class TestEdgeCases:
    """Test edge cases for win detection."""

    def test_full_board_no_winner(self):
        """Test win detection on a full board."""
        state = GameState()
        # Create a realistic board pattern with mixed pieces
        # Just verify that a full, arbitrary board doesn't crash win detection
        import random
        random.seed(42)
        for row in range(6):
            for col in range(7):
                state.set_cell(row, col, random.choice([0, 1]))

        # This should not crash, and may or may not have a winner
        # The important thing is it handles a full board
        state.current_player = 0
        _check_win(state)  # Should not raise an exception

    def test_win_at_board_edge(self):
        """4-in-a-row at board edges should be detected."""
        state = GameState()
        # Top row
        for col in range(4):
            state.set_cell(0, col, 0)

        state.current_player = 0
        assert _check_win(state)

    def test_win_at_right_edge(self):
        """4-in-a-row at right edge should be detected."""
        state = GameState()
        for col in range(3, 7):
            state.set_cell(5, col, 1)

        state.current_player = 1
        assert _check_win(state)
