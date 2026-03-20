"""
Tests for removal moves and gravity mechanics.
"""

import pytest
from connect4.board import GameState
from connect4.moves import PlacementMove, RemovalMove
from connect4.rules import apply_move


class TestRemovalBasics:
    """Test basic removal functionality."""

    def test_remove_single_piece(self):
        """Should remove a piece from the board."""
        state = GameState()
        state.set_cell(3, 3, 0)

        move = RemovalMove(row=3, col=3)
        new_state = apply_move(state, move)

        assert new_state.get_cell(3, 3) is None

    def test_removal_decrements_counter(self):
        """Removal should decrement player's removal count."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.current_player = 0

        assert state.removals_remaining[0] == 2

        move = RemovalMove(row=0, col=0)
        new_state = apply_move(state, move)

        assert new_state.removals_remaining[0] == 1

    def test_removal_switches_player(self):
        """Removal should switch to next player."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.current_player = 0

        move = RemovalMove(row=0, col=0)
        new_state = apply_move(state, move)

        assert new_state.current_player == 1


class TestGravity:
    """Test gravity after piece removal."""

    def test_gravity_simple(self):
        """Pieces above removed piece should fall down."""
        state = GameState()
        # Stack: [1] at row 3, [0] at row 4, empty at row 5
        state.set_cell(3, 0, 1)
        state.set_cell(4, 0, 0)

        # Remove player 0's piece at row 4
        move = RemovalMove(row=4, col=0)
        new_state = apply_move(state, move)

        # Piece at row 3 should now be at row 4
        assert new_state.get_cell(3, 0) is None
        assert new_state.get_cell(4, 0) == 1

    def test_gravity_multiple_pieces(self):
        """Multiple pieces should cascade down."""
        state = GameState()
        # Fill column 0: [0] at 2, [1] at 3, [0] at 4
        state.set_cell(2, 0, 0)
        state.set_cell(3, 0, 1)
        state.set_cell(4, 0, 0)

        # Remove piece at row 3
        move = RemovalMove(row=3, col=0)
        new_state = apply_move(state, move)

        # Piece at 2 should be at 3, piece at 4 should be at 4 (no cascading further)
        assert new_state.get_cell(2, 0) is None
        assert new_state.get_cell(3, 0) == 0
        assert new_state.get_cell(4, 0) == 0

    def test_gravity_only_affects_removed_column(self):
        """Gravity should only apply to the column with removed piece."""
        state = GameState()
        state.set_cell(4, 0, 0)  # Column 0, row 4
        state.set_cell(4, 1, 1)  # Column 1, row 4 (unaffected)

        move = RemovalMove(row=4, col=0)
        new_state = apply_move(state, move)

        # Column 0: piece removed, row 4 now empty
        assert new_state.get_cell(4, 0) is None

        # Column 1: piece should still be at row 4
        assert new_state.get_cell(4, 1) == 1

    def test_gravity_with_gap(self):
        """Gravity pulls pieces above the gap downward."""
        state = GameState()
        # Column: [0] at 1, empty at 2, [1] at 3, [0] at 4
        state.set_cell(1, 0, 0)
        state.set_cell(3, 0, 1)
        state.set_cell(4, 0, 0)

        # Remove piece at row 3
        move = RemovalMove(row=3, col=0)
        new_state = apply_move(state, move)

        # Piece at row 1 (above gap) falls to row 2
        assert new_state.get_cell(1, 0) is None
        assert new_state.get_cell(2, 0) == 0
        # Piece at row 4 (below gap) stays
        assert new_state.get_cell(4, 0) == 0

    def test_gravity_bottom_row(self):
        """Removing bottom piece: no gravity effect (pieces above fall)."""
        state = GameState()
        state.set_cell(4, 0, 0)
        state.set_cell(5, 0, 1)  # Bottom

        move = RemovalMove(row=5, col=0)
        new_state = apply_move(state, move)

        # Piece at 4 falls to 5
        assert new_state.get_cell(4, 0) is None
        assert new_state.get_cell(5, 0) == 0

    def test_gravity_top_row(self):
        """Removing top piece: no pieces to fall."""
        state = GameState()
        state.set_cell(0, 0, 0)

        move = RemovalMove(row=0, col=0)
        new_state = apply_move(state, move)

        assert new_state.get_cell(0, 0) is None


class TestRemovalRealistic:
    """Test removal in more realistic game scenarios."""

    def test_removal_after_placement_sequence(self):
        """Remove a piece after some placements."""
        state = GameState()

        # Place a few pieces
        state = apply_move(state, PlacementMove(column=3))  # P0 at (5,3)
        state = apply_move(state, PlacementMove(column=3))  # P1 at (4,3)
        state = apply_move(state, PlacementMove(column=3))  # P0 at (3,3)

        # P1's turn, still has removals
        assert state.current_player == 1
        assert state.removals_remaining[1] == 2

        # Remove P0's top piece at (3, 3)
        state = apply_move(state, RemovalMove(row=3, col=3))

        # P1's piece should still be at (4,3), no pieces above gap to fall down
        assert state.get_cell(3, 3) is None
        assert state.get_cell(4, 3) == 1  # P1's piece stays

        # P1's removals decremented
        assert state.removals_remaining[1] == 1
        assert state.current_player == 0

    def test_two_removals_per_player(self):
        """Players should be limited to 2 removals."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.set_cell(1, 1, 0)
        state.set_cell(2, 2, 0)

        # Use 2 removals for player 0
        state = apply_move(state, RemovalMove(row=0, col=0))
        state = apply_move(state, PlacementMove(column=0))  # P1's move
        state = apply_move(state, RemovalMove(row=1, col=1))

        assert state.removals_remaining[0] == 0

        # P1's turn, but P0 now can't remove
        state = apply_move(state, PlacementMove(column=0))
        assert state.current_player == 0

        # Check that removal moves are not available for P0
        from connect4.rules import get_legal_moves
        from connect4.moves import RemovalMove as RM

        legal = get_legal_moves(state)
        removal_moves = [m for m in legal if isinstance(m, RM)]
        assert len(removal_moves) == 0


class TestRemovalAndWin:
    """Test removal interactions with win conditions."""

    def test_removal_enabling_win(self):
        """Removing opponent piece might allow own win next turn (unlikely but possible)."""
        state = GameState()
        # Set up: P0 has [X,X,X, ], P1 blocking with [O] in gap
        state.set_cell(5, 0, 0)
        state.set_cell(5, 1, 0)
        state.set_cell(5, 2, 1)
        state.set_cell(5, 3, 0)

        state.current_player = 0
        # P0 removes P1's blocking piece
        state = apply_move(state, RemovalMove(row=5, col=2))

        # P1's turn
        assert state.current_player == 1

        # Now if P0 gets another turn, they could win by placing in col 2
        # (but that's P1's turn next)
