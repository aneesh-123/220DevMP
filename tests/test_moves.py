"""
Tests for move representation and legal move generation.
"""

import pytest
from connect4.board import GameState
from connect4.moves import PlacementMove, RemovalMove
from connect4.rules import get_legal_moves, apply_move


class TestMoveTypes:
    """Test move dataclass behaviors."""

    def test_placement_move(self):
        """PlacementMove should store column."""
        move = PlacementMove(column=3)
        assert move.column == 3
        assert str(move) == "Place in column 3"

    def test_removal_move(self):
        """RemovalMove should store row and col."""
        move = RemovalMove(row=2, col=4)
        assert move.row == 2
        assert move.col == 4
        assert str(move) == "Remove from (2, 4)"

    def test_moves_are_frozen(self):
        """Moves should be immutable (frozen dataclasses)."""
        move1 = PlacementMove(column=3)
        move2 = PlacementMove(column=3)

        # Should be hashable and comparable
        assert move1 == move2
        assert hash(move1) == hash(move2)


class TestLegalPlacementMoves:
    """Test placement move generation."""

    def test_all_columns_legal_on_empty_board(self):
        """All 7 columns should be legal on empty board."""
        state = GameState()
        moves = get_legal_moves(state)

        placement_moves = [m for m in moves if isinstance(m, PlacementMove)]
        assert len(placement_moves) == 7

        for col in range(7):
            assert any(m.column == col for m in placement_moves)

    def test_full_column_not_legal(self):
        """Placement in full column should not be legal."""
        state = GameState()
        # Fill first column
        for row in range(6):
            state.set_cell(row, 0, 0)

        moves = get_legal_moves(state)
        placement_moves = [m for m in moves if isinstance(m, PlacementMove)]

        assert not any(m.column == 0 for m in placement_moves)
        assert any(m.column == 1 for m in placement_moves)

    def test_placement_moves_legal_before_removal_moves(self):
        """Even if removals available, placement moves exist first."""
        state = GameState()
        moves = get_legal_moves(state)

        # Should have 7 placement + 0 removal (no pieces to remove yet)
        assert len(moves) == 7


class TestLegalRemovalMoves:
    """Test removal move generation."""

    def test_no_removal_moves_on_empty_board(self):
        """No removal moves when board is empty."""
        state = GameState()
        removal_moves = [m for m in get_legal_moves(state) if isinstance(m, RemovalMove)]
        assert len(removal_moves) == 0

    def test_removal_moves_when_pieces_exist(self):
        """Should generate removal move for each occupied cell."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.set_cell(1, 1, 1)
        state.set_cell(2, 2, 0)

        removal_moves = [m for m in get_legal_moves(state) if isinstance(m, RemovalMove)]
        assert len(removal_moves) == 3

        assert RemovalMove(row=0, col=0) in removal_moves
        assert RemovalMove(row=1, col=1) in removal_moves
        assert RemovalMove(row=2, col=2) in removal_moves

    def test_no_removal_moves_when_none_remaining(self):
        """No removal moves if player has used all removals."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.removals_remaining[0] = 0  # Player 0 out of removals

        removal_moves = [m for m in get_legal_moves(state) if isinstance(m, RemovalMove)]
        assert len(removal_moves) == 0

    def test_removal_moves_only_for_current_player(self):
        """Removal moves should only exist if current player has removals left."""
        state = GameState()
        state.set_cell(0, 0, 0)
        state.current_player = 1
        state.removals_remaining[1] = 0

        removal_moves = [m for m in get_legal_moves(state) if isinstance(m, RemovalMove)]
        assert len(removal_moves) == 0


class TestApplyPlacementMove:
    """Test applying placement moves."""

    def test_placement_lands_at_bottom(self):
        """Piece should fall to bottom of column."""
        state = GameState()
        move = PlacementMove(column=3)
        new_state = apply_move(state, move)

        # Piece should land at row 5 (bottom)
        assert new_state.get_cell(5, 3) == 0

    def test_placement_stacks_pieces(self):
        """Multiple pieces in same column should stack."""
        state = GameState()
        state = apply_move(state, PlacementMove(column=3))  # row 5
        state = apply_move(state, PlacementMove(column=3))  # row 4, player 1

        assert state.get_cell(5, 3) == 0
        assert state.get_cell(4, 3) == 1

    def test_placement_switches_player(self):
        """Active player should switch after placement."""
        state = GameState()
        assert state.current_player == 0
        new_state = apply_move(state, PlacementMove(column=0))
        assert new_state.current_player == 1

    def test_placement_adds_to_history(self):
        """Move should be added to history."""
        state = GameState()
        move = PlacementMove(column=2)
        new_state = apply_move(state, move)
        assert move in new_state.move_history
