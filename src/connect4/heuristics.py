from .board import GameState


def terminal_state_bonus(state: GameState, player: int) -> float:
    """Return a large score for terminal win/loss states, else 0."""
    if not state.is_terminal:
        return 0

    if state.winner == player:
        return 10000
    elif state.winner is not None:
        return -10000
    else:
        return 0
