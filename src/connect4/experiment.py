"""
Experiment runner for Connect 4.

Allows running multiple games between two bots and collecting statistics.
"""

import random
from dataclasses import dataclass
from typing import Optional, Any
from .board import GameState
from .rules import get_legal_moves, apply_move


@dataclass
class ExperimentResults:
    """
    Results from running a batch of games.

    Attributes:
        bot1_wins: Number of games won by bot1.
        bot2_wins: Number of games won by bot2.
        draws: Number of games that ended in a draw (board full).
        total_games: Total number of games played.
        bot1_name: Name of bot1.
        bot2_name: Name of bot2.
    """

    bot1_wins: int
    bot2_wins: int
    draws: int
    total_games: int
    bot1_name: str
    bot2_name: str

    def __str__(self) -> str:
        lines = [
            f"\n{'=' * 50}",
            f"Experiment Results",
            f"{'=' * 50}",
            f"{self.bot1_name} vs {self.bot2_name}",
            f"Total games: {self.total_games}",
            f"",
            f"{self.bot1_name:20} wins: {self.bot1_wins}  ({100*self.bot1_wins/self.total_games:.1f}%)",
            f"{self.bot2_name:20} wins: {self.bot2_wins}  ({100*self.bot2_wins/self.total_games:.1f}%)",
            f"{'Draws':20}: {self.draws}  ({100*self.draws/self.total_games:.1f}%)",
            f"{'=' * 50}",
        ]
        return "\n".join(lines)


class Experiment:
    """
    Run a series of games between two bots and collect statistics.
    """

    def __init__(
        self,
        bot1: Any,
        bot2: Any,
        num_games: int = 10,
        verbose: bool = False,
        seed: Optional[int] = None,
        alternate_starts: bool = True,
        bot1_name: str = "",
        bot2_name: str = "",
    ):
        """
        Initialize an experiment.

        Args:
            bot1: The first bot object.
            bot2: The second bot object.
            num_games: Number of games to play.
            verbose: If True, print progress updates.
            seed: Random seed for reproducibility.
            alternate_starts: If True, alternate who goes first each game.
                             If False, bot1 always goes first.
            bot1_name: Display name for bot1.
            bot2_name: Display name for bot2.
        """
        self.bot1 = bot1
        self.bot2 = bot2
        self.num_games = num_games
        self.verbose = verbose
        self.seed = seed
        self.alternate_starts = alternate_starts
        self.bot1_name = bot1_name or str(bot1)
        self.bot2_name = bot2_name or str(bot2)

        if seed is not None:
            random.seed(seed)

    def run(self) -> ExperimentResults:
        """
        Run the experiment: play num_games games and return results.

        Returns:
            ExperimentResults with win/draw counts.
        """
        bot1_wins = 0
        bot2_wins = 0
        draws = 0

        for game_num in range(self.num_games):
            # Decide who goes first
            if self.alternate_starts:
                bot1_is_player0 = (game_num % 2 == 0)
            else:
                bot1_is_player0 = True

            winner = self._play_one_game(bot1_is_player0)

            if winner == 0:
                if bot1_is_player0:
                    bot1_wins += 1
                else:
                    bot2_wins += 1
            elif winner == 1:
                if bot1_is_player0:
                    bot2_wins += 1
                else:
                    bot1_wins += 1
            else:
                draws += 1

            if self.verbose:
                starter = self.bot1_name if bot1_is_player0 else self.bot2_name

                if winner is None:
                    result_str = "Draw"
                else:
                    winner_name = (
                        self.bot1_name
                        if (winner == 0 and bot1_is_player0)
                        or (winner == 1 and not bot1_is_player0)
                        else self.bot2_name
                    )
                    result_str = f"{winner_name} wins"

                print(f"  Game {game_num + 1}/{self.num_games}: {result_str} (first move: {starter})")

        results = ExperimentResults(
            bot1_wins=bot1_wins,
            bot2_wins=bot2_wins,
            draws=draws,
            total_games=self.num_games,
            bot1_name=self.bot1_name,
            bot2_name=self.bot2_name,
        )
        return results

    def _play_one_game(self, bot1_is_player0: bool) -> Optional[int]:
        """
        Play a single game and return the winner (0, 1) or None for draw.

        Args:
            bot1_is_player0: If True, bot1 is player 0; else bot1 is player 1.

        Returns:
            Winner (0 or 1) or None if draw.
        """
        state = GameState()

        while not state.is_terminal:
            legal_moves = get_legal_moves(state)
            if not legal_moves:
                # No moves available (shouldn't happen in Connect 4)
                break

            # Determine which bot plays this turn
            if state.current_player == 0:
                if bot1_is_player0:
                    move = self.bot1.choose_move(state)
                else:
                    move = self.bot2.choose_move(state)
            else:
                if bot1_is_player0:
                    move = self.bot2.choose_move(state)
                else:
                    move = self.bot1.choose_move(state)

            if move is None:
                break

            state = apply_move(state, move)

        return state.winner
