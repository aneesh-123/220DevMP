"""
Minimax search with alpha-beta pruning for Connect 4.

This search implementation is provided as infrastructure.
Students will plug in their own evaluation functions via the 'evaluator' parameter.
"""

from typing import Optional, Tuple, Any
from .board import GameState
from .rules import get_legal_moves, apply_move


class SearchResult:
    """
    Result of a minimax search.

    Attributes:
        best_move: The best move found at the root.
        score: The score of the best move from the perspective of the root player.
        depth_searched: The depth that was searched.
    """

    def __init__(self, best_move: Any, score: float, depth_searched: int):
        self.best_move = best_move
        self.score = score
        self.depth_searched = depth_searched

    def __repr__(self) -> str:
        return f"SearchResult(move={self.best_move}, score={self.score:.2f})"


def minimax(
    state: GameState,
    depth: int,
    evaluator: Any,
    root_player: int,
    maximizing: bool = True,
    alpha: float = float('-inf'),
    beta: float = float('inf'),
) -> Tuple[Optional[Any], float]:
    """
    Minimax search with alpha-beta pruning.

    Args:
        state: The current game state.
        depth: How many moves ahead to search. 0 means use evaluator on current state.
        evaluator: An object with an evaluate(state, player) method.
        root_player: The player whose perspective all leaf scores use.
        maximizing: True if we're maximizing (searching for the root player's move).
        alpha: Alpha pruning threshold.
        beta: Beta pruning threshold.

    Returns:
        (best_move, score) tuple.
        - best_move is the best move found, or None if no moves available.
        - score is the minimax score of that move.
    """
    # Terminal node: evaluate the position
    if depth == 0 or state.is_terminal:
        score = evaluator.evaluate(state, root_player)
        return None, score

    legal_moves = get_legal_moves(state)
    if not legal_moves:
        score = evaluator.evaluate(state, root_player)
        return None, score

    best_move = None

    if maximizing:
        max_eval = float('-inf')
        for move in legal_moves:
            next_state = apply_move(state, move)
            _, eval_score = minimax(
                next_state,
                depth - 1,
                evaluator,
                root_player,
                maximizing=False,
                alpha=alpha,
                beta=beta,
            )
            if eval_score > max_eval:
                max_eval = eval_score
                best_move = move
            alpha = max(alpha, eval_score)
            if beta <= alpha:
                break  # Beta cutoff
        return best_move, max_eval
    else:
        min_eval = float('inf')
        for move in legal_moves:
            next_state = apply_move(state, move)
            _, eval_score = minimax(
                next_state,
                depth - 1,
                evaluator,
                root_player,
                maximizing=True,
                alpha=alpha,
                beta=beta,
            )
            if eval_score < min_eval:
                min_eval = eval_score
                best_move = move
            beta = min(beta, eval_score)
            if beta <= alpha:
                break  # Alpha cutoff
        return best_move, min_eval


def search(
    state: GameState,
    depth: int,
    evaluator: Any,
) -> SearchResult:
    """
    Entry point for minimax search from the current state.

    Args:
        state: The current game state.
        depth: Search depth.
        evaluator: An object with an evaluate(state, player) method.

    Returns:
        SearchResult with best_move, score, and depth_searched.
    """
    root_player = state.current_player
    best_move, score = minimax(
        state,
        depth,
        evaluator,
        root_player,
        maximizing=True,
        alpha=float('-inf'),
        beta=float('inf'),
    )
    return SearchResult(best_move, score, depth)
