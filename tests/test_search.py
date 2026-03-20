"""
Tests for minimax search and bot decision-making.
"""

import pytest
from connect4.board import GameState
from connect4.moves import PlacementMove, RemovalMove
from connect4.search import search, SearchResult
from connect4.heuristics import BasicEvaluator
from connect4.bots import MinimaxBot
from connect4.rules import apply_move, get_legal_moves


class TestSearchBasics:
    """Test basic search functionality."""

    def test_search_returns_move(self):
        """Search should return a legal move."""
        state = GameState()
        evaluator = BasicEvaluator()

        result = search(state, depth=2, evaluator=evaluator)

        assert result.best_move is not None
        assert isinstance(result.best_move, (PlacementMove, RemovalMove))

    def test_search_returns_legal_move(self):
        """Search result should be in legal moves."""
        state = GameState()
        evaluator = BasicEvaluator()

        result = search(state, depth=2, evaluator=evaluator)
        legal_moves = get_legal_moves(state)

        assert result.best_move in legal_moves

    def test_search_depth_zero(self):
        """Depth 0 search should handle terminal or empty states."""
        state = GameState()
        evaluator = BasicEvaluator()

        result = search(state, depth=0, evaluator=evaluator)

        # Depth 0 evaluates immediately, returns None for move
        assert result.best_move is None
        assert isinstance(result.score, (int, float))


class TestSearchWinDetection:
    """Test that search recognizes winning positions."""

    def test_search_finds_immediate_win(self):
        """Search should find a winning move if available."""
        state = GameState()
        evaluator = BasicEvaluator()

        # Set up board where player 0 can win by placing in column 3
        state.set_cell(5, 0, 0)
        state.set_cell(5, 1, 0)
        state.set_cell(5, 2, 0)

        state.current_player = 0
        result = search(state, depth=1, evaluator=evaluator)

        # Minimax should find the winning move
        assert result.best_move == PlacementMove(column=3)

    def test_search_evaluates_win_highly(self):
        """Positions with wins should have high scores."""
        # State where player 0 has already won
        state = GameState()
        for col in range(4):
            state.set_cell(5, col, 0)

        state.is_terminal = True
        state.winner = 0
        state.current_player = 0

        evaluator = BasicEvaluator()
        score = evaluator.evaluate(state, player=0)

        # Should be very high (win bonus)
        assert score > 1000

    def test_search_evaluates_loss_lowly(self):
        """Positions where opponent won should have low scores."""
        state = GameState()
        for col in range(4):
            state.set_cell(5, col, 1)

        state.is_terminal = True
        state.winner = 1
        state.current_player = 0

        evaluator = BasicEvaluator()
        score = evaluator.evaluate(state, player=0)

        # Should be very low (loss penalty)
        assert score < -1000


class TestBotChoosing:
    """Test bot decision-making."""

    def test_minimax_bot_chooses_legal_move(self):
        """MinimaxBot should choose a legal move."""
        state = GameState()
        evaluator = BasicEvaluator()
        bot = MinimaxBot(evaluator=evaluator, depth=2)

        move = bot.choose_move(state)

        assert move is not None
        legal_moves = get_legal_moves(state)
        assert move in legal_moves

    def test_minimax_bot_finds_winning_move(self):
        """MinimaxBot should find a winning move if available."""
        state = GameState()

        # Set up a winning position
        state.set_cell(5, 0, 0)
        state.set_cell(5, 1, 0)
        state.set_cell(5, 2, 0)
        state.current_player = 0

        evaluator = BasicEvaluator()
        bot = MinimaxBot(evaluator=evaluator, depth=1)

        move = bot.choose_move(state)

        assert move == PlacementMove(column=3)


class TestSearchDepth:
    """Test search behavior at different depths."""

    def test_greater_depth_considers_more_moves(self):
        """Higher depth should make better decisions (usually)."""
        state = GameState()
        evaluator = BasicEvaluator()

        # Just verify both depths return valid moves
        result_d1 = search(state, depth=1, evaluator=evaluator)
        result_d2 = search(state, depth=2, evaluator=evaluator)

        assert result_d1.best_move is not None
        assert result_d2.best_move is not None


class TestMiniverbByRemoval:
    """Test search with removal moves."""

    def test_search_considers_removal_moves(self):
        """Search should consider removal moves when available."""
        state = GameState()
        # Place a piece that search might want to remove
        state.set_cell(5, 3, 1)  # Opponent's piece
        state.current_player = 0

        evaluator = BasicEvaluator()
        result = search(state, depth=1, evaluator=evaluator)

        # Search should find a move (placement or removal)
        assert result.best_move is not None
        legal_moves = get_legal_moves(state)
        assert result.best_move in legal_moves


class TestSearchReproducibility:
    """Test that search with same state produces same result."""

    def test_search_deterministic(self):
        """Same state and parameters should produce same move."""
        state = GameState()
        evaluator = BasicEvaluator()

        result1 = search(state, depth=2, evaluator=evaluator)
        result2 = search(state, depth=2, evaluator=evaluator)

        assert result1.best_move == result2.best_move
        assert result1.score == result2.score
