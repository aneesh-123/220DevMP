"""Reusable heuristic helper functions for evaluator design."""

from typing import Iterable, List, Tuple

from .board import GameState
from .moves import PlacementMove
from .rules import apply_move, get_legal_moves


def terminal_state_bonus(state: GameState, player: int) -> float:
    """Return a large score for terminal win/loss states, else 0."""
    if not state.is_terminal:
        return 0

    if state.winner == player:
        return 10000
    if state.winner is not None:
        return -10000
    return 0


def center_control_score(state: GameState, player: int) -> float:
    """Reward control of the center columns."""
    opponent = 1 - player
    weights = [1, 2, 3, 4, 3, 2, 1]
    score = 0.0

    for row in range(GameState.ROWS):
        for col in range(GameState.COLS):
            cell = state.get_cell(row, col)
            if cell == player:
                score += weights[col]
            elif cell == opponent:
                score -= weights[col]

    return score


def window_pattern_score(state: GameState, player: int) -> float:
    """Score every four-cell window based on how favorable it looks."""
    opponent = 1 - player
    score = 0.0

    for window in _iter_windows(state):
        player_count = window.count(player)
        opponent_count = window.count(opponent)
        empty_count = window.count(None)

        if player_count > 0 and opponent_count > 0:
            continue

        if player_count == 4:
            score += 1000
        elif player_count == 3 and empty_count == 1:
            score += 60
        elif player_count == 2 and empty_count == 2:
            score += 12

        if opponent_count == 4:
            score -= 1000
        elif opponent_count == 3 and empty_count == 1:
            score -= 70
        elif opponent_count == 2 and empty_count == 2:
            score -= 12

    return score


def three_in_row_score(state: GameState, player: int) -> float:
    """Count all open three-in-a-row patterns, playable or not."""
    opponent = 1 - player
    score = 0.0

    for window in _iter_windows(state):
        player_count = window.count(player)
        opponent_count = window.count(opponent)
        empty_count = window.count(None)

        if player_count == 3 and opponent_count == 0 and empty_count == 1:
            score += 80
        elif opponent_count == 3 and player_count == 0 and empty_count == 1:
            score -= 80

    return score


def playable_three_in_row_score(state: GameState, player: int) -> float:
    """Reward only open threes whose empty square can be played now."""
    opponent = 1 - player
    score = 0.0

    for coords, window in _iter_windows_with_coords(state):
        player_count = window.count(player)
        opponent_count = window.count(opponent)
        empty_count = window.count(None)

        if empty_count != 1:
            continue

        empty_index = window.index(None)
        empty_row, empty_col = coords[empty_index]
        playable = _is_playable_cell(state, empty_row, empty_col)

        if not playable:
            continue

        if player_count == 3 and opponent_count == 0:
            score += 140
        elif opponent_count == 3 and player_count == 0:
            score -= 160

    return score


def immediate_win_and_block_score(state: GameState, player: int) -> float:
    """Reward immediate winning moves and penalize immediate opponent wins."""
    player_wins = _count_immediate_wins(state, player)
    opponent_wins = _count_immediate_wins(state, 1 - player)
    return 500 * player_wins - 650 * opponent_wins


def _iter_windows(state: GameState) -> Iterable[List[int]]:
    """Yield every horizontal, vertical, and diagonal four-cell window."""
    for _, window in _iter_windows_with_coords(state):
        yield window


def _iter_windows_with_coords(
    state: GameState,
) -> Iterable[Tuple[List[Tuple[int, int]], List[int]]]:
    """Yield every four-cell window along with its coordinates."""
    directions = ((0, 1), (1, 0), (1, 1), (1, -1))

    for row in range(GameState.ROWS):
        for col in range(GameState.COLS):
            for d_row, d_col in directions:
                coords = []
                window = []
                for step in range(4):
                    next_row = row + step * d_row
                    next_col = col + step * d_col
                    if not (0 <= next_row < GameState.ROWS and 0 <= next_col < GameState.COLS):
                        break
                    coords.append((next_row, next_col))
                    window.append(state.get_cell(next_row, next_col))
                if len(window) == 4:
                    yield coords, window


def _is_playable_cell(state: GameState, row: int, col: int) -> bool:
    """Return True when a piece could land on this empty cell immediately."""
    if state.get_cell(row, col) is not None:
        return False
    if row == GameState.ROWS - 1:
        return True
    return state.get_cell(row + 1, col) is not None


def _count_immediate_wins(state: GameState, player: int) -> int:
    """Count legal placement moves that win immediately for a player."""
    candidate = state.copy()
    candidate.current_player = player
    wins = 0

    for move in get_legal_moves(candidate):
        if not isinstance(move, PlacementMove):
            continue
        next_state = apply_move(candidate, move)
        if next_state.is_terminal and next_state.winner == player:
            wins += 1

    return wins
