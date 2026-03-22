"""
Experiment runner for Connect 4.

Includes:
- play_one_game: Play a single game between two bots from a given state.
- ExperimentSuite: Run a student evaluator against all test evaluators
  across all board states.
"""

from typing import Optional, Any, List, Tuple, Dict
from .board import GameState
from .bots import MinimaxBot
from .rules import get_legal_moves, apply_move


def play_one_game(
    bot_a: MinimaxBot,
    bot_b: MinimaxBot,
    initial_state: Optional[GameState] = None,
) -> Optional[int]:
    """
    Play a single game and return the winner.

    bot_a plays as player 0 (whoever current_player is in initial_state).
    bot_b plays as player 1.

    Returns:
        Winner (0 or 1) or None if draw.
    """
    state = initial_state.copy() if initial_state else GameState()

    while not state.is_terminal:
        legal_moves = get_legal_moves(state)
        if not legal_moves:
            break

        if state.current_player == 0:
            move = bot_a.choose_move(state)
        else:
            move = bot_b.choose_move(state)

        if move is None:
            break

        state = apply_move(state, move)

    return state.winner


class ExperimentSuite:
    """
    Run a student evaluator against all test evaluators on all board states.

    For each (test_evaluator, board_state) pair, plays 2 games:
    one with the student going first, one with the opponent going first.
    """

    def __init__(
        self,
        evaluator: Any,
        evaluator_name: str,
        test_evaluators: Dict[str, Any],
        board_states: List[Tuple[str, GameState]],
        depth: int = 4,
    ):
        self.evaluator = evaluator
        self.evaluator_name = evaluator_name
        self.test_evaluators = test_evaluators
        self.board_states = board_states
        self.depth = depth

    def run(self):
        """Run all matches and print results in real-time."""
        num_opponents = len(self.test_evaluators)
        num_boards = len(self.board_states)
        total_games = num_opponents * num_boards * 2

        print(f"\nTesting: {self.evaluator_name}")
        print(f"{num_opponents} opponents x {num_boards} board states x 2 sides = {total_games} games")
        print("=" * 60)

        student_bot = MinimaxBot(evaluator=self.evaluator, depth=self.depth)

        # Track per-opponent results
        opponent_results = {}
        total_wins = 0
        total_losses = 0
        total_draws = 0
        game_number = 0

        for opp_name, opp_evaluator in self.test_evaluators.items():
            opp_bot = MinimaxBot(evaluator=opp_evaluator, depth=self.depth)
            opp_wins = 0
            opp_losses = 0
            opp_draws = 0

            print(f"\n  vs {opp_name}:")

            for board_name, board_state in self.board_states:
                results = []

                for student_goes_first in [True, False]:
                    game_number += 1
                    side = "O" if student_goes_first else "X"

                    if student_goes_first:
                        winner = play_one_game(student_bot, opp_bot, board_state)
                        student_won = (winner == 0)
                        opp_won = (winner == 1)
                    else:
                        winner = play_one_game(opp_bot, student_bot, board_state)
                        student_won = (winner == 1)
                        opp_won = (winner == 0)

                    if student_won:
                        results.append(f"Win as {side}")
                        opp_wins += 1
                    elif opp_won:
                        results.append(f"Loss as {side}")
                        opp_losses += 1
                    else:
                        results.append(f"Draw as {side}")
                        opp_draws += 1

                print(f"    {board_name}: {results[0]}, {results[1]}")

            total_wins += opp_wins
            total_losses += opp_losses
            total_draws += opp_draws
            opp_total = opp_wins + opp_losses + opp_draws
            opp_pct = 100 * opp_wins / opp_total if opp_total else 0
            opponent_results[opp_name] = (opp_wins, opp_losses, opp_draws)
            print(f"    Subtotal: W:{opp_wins} L:{opp_losses} D:{opp_draws} ({opp_pct:.0f}% wins)")

        # Final summary
        print("\n" + "=" * 60)
        print(f"EXPERIMENT RESULTS: {self.evaluator_name}")
        print("=" * 60)

        for opp_name, (w, l, d) in opponent_results.items():
            opp_total = w + l + d
            pct = 100 * w / opp_total if opp_total else 0
            print(f"  vs {opp_name:20s}  W:{w:2d}  L:{l:2d}  D:{d:2d}  ({pct:.0f}%)")

        print("-" * 60)
        grand_total = total_wins + total_losses + total_draws
        win_pct = 100 * total_wins / grand_total if grand_total else 0
        print(f"  {'TOTAL':23s}  W:{total_wins:2d}  L:{total_losses:2d}  D:{total_draws:2d}  ({win_pct:.0f}%)")
        print("=" * 60)

        print(f"\nGRADE GUIDE:")
        print(f"  70%+ win rate: Excellent heuristic design")
        print(f"  50%+ win rate: Good heuristic design")
        print(f"  30%+ win rate: Needs improvement")
        print(f"  <30% win rate: Significant issues")
