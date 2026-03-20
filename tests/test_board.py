"""
Tests for board representation and basic state management.
"""

import pytest
from connect4.board import GameState
from connect4.moves import PlacementMove, RemovalMove


class TestBoardInitialization:
    """Test initial board state."""

    def test_board_size(self):
        """Board should be 6 rows by 7 columns."""
        state = GameState()
        assert len(state.board) == 6
        assert all(len(row) == 7 for row in state.board)

    def test_board_empty(self):
        """Initial board should be empty."""
        state = GameState()
        for row in state.board:
            for cell in row:
                assert cell is None

    def test_current_player_is_zero(self):
        """Player 0 should go first."""
        state = GameState()
        assert state.current_player == 0

    def test_removals_initialized(self):
        """Each player should start with 2 removals."""
        state = GameState()
        assert state.removals_remaining == [2, 2]

    def test_not_terminal(self):
        """Game should not be terminal at start."""
        state = GameState()
        assert not state.is_terminal
        assert state.winner is None


class TestBoardMutations:
    """Test setting and getting board cells."""

    def test_set_get_cell(self):
        """Should be able to set and get cells."""
        state = GameState()
        state.set_cell(0, 0, 0)
        assert state.get_cell(0, 0) == 0

    def test_set_cell_out_of_bounds(self):
        """Setting out of bounds should not crash."""
        state = GameState()
        state.set_cell(-1, 0, 0)
        state.set_cell(0, 7, 0)
        state.set_cell(6, 0, 0)
        # Should not raise an error

    def test_get_cell_out_of_bounds(self):
        """Getting out of bounds should return None."""
        state = GameState()
        assert state.get_cell(-1, 0) is None
        assert state.get_cell(0, 7) is None
        assert state.get_cell(6, 0) is None

    def test_is_cell_occupied(self):
        """Should detect occupied and empty cells."""
        state = GameState()
        assert not state.is_cell_occupied(0, 0)
        state.set_cell(0, 0, 0)
        assert state.is_cell_occupied(0, 0)


class TestColumnHeight:
    """Test column height tracking."""

    def test_empty_column_height(self):
        """Empty column should have height 0."""
        state = GameState()
        for col in range(7):
            assert state.get_column_height(col) == 0

    def test_column_height_with_pieces(self):
        """Column height should match number of pieces."""
        state = GameState()
        # Fill a column from bottom up
        for row in range(5, -1, -1):
            state.set_cell(row, 0, 0)
            expected_height = 6 - row
            assert state.get_column_height(0) == expected_height

    def test_column_full(self):
        """Full column should be detected."""
        state = GameState()
        for row in range(6):
            assert not state.is_column_full(0)
            state.set_cell(row, 0, 0)
        assert state.is_column_full(0)


class TestBoardCopy:
    """Test state copying."""

    def test_copy_creates_new_instance(self):
        """Copy should create a new GameState object."""
        state1 = GameState()
        state1.set_cell(0, 0, 0)
        state2 = state1.copy()

        assert state1 is not state2
        assert state1.get_cell(0, 0) == state2.get_cell(0, 0)

    def test_copy_is_independent(self):
        """Modifying copy should not affect original."""
        state1 = GameState()
        state2 = state1.copy()
        state2.set_cell(0, 0, 1)

        assert state1.get_cell(0, 0) is None
        assert state2.get_cell(0, 0) == 1

    def test_copy_board_independence(self):
        """Board list should be deep copied."""
        state1 = GameState()
        state1.set_cell(3, 3, 1)
        state2 = state1.copy()

        state2.set_cell(3, 3, 0)
        assert state1.get_cell(3, 3) == 1
        assert state2.get_cell(3, 3) == 0


class TestPieceCount:
    """Test piece counting."""

    def test_count_pieces_empty_board(self):
        """Empty board should have 0 pieces for each player."""
        state = GameState()
        assert state.count_pieces(0) == 0
        assert state.count_pieces(1) == 0

    def test_count_pieces(self):
        """Should count pieces correctly."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.set_cell(1, 1, 0)
        state.set_cell(2, 2, 1)

        assert state.count_pieces(0) == 2
        assert state.count_pieces(1) == 1
