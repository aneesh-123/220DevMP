"""
Simple CLI and runner for Connect 4.

Provides:
- Interactive play against MinimaxBot
- Batch experiments comparing bots
- Simple command-line interface
"""

from .board import GameState
from .bots import MinimaxBot
from .heuristics import BasicEvaluator, StudentEvaluator
from .experiment import Experiment
from .rules import get_legal_moves, apply_move


def play_interactive_game(depth: int = 5):
    """
    Play an interactive game against a MinimaxBot.

    The human is player 0 (O), the bot is player 1 (X).

    Args:
        depth: Search depth for minimax (default 5).
    """
    state = GameState()
    bot = MinimaxBot(evaluator=BasicEvaluator(), depth=depth)

    print("\n" + state.display())
    print(f"\nYou are player 0 (O). Playing against {bot}.")
    print("Commands:")
    print("  place <column>    - Drop a piece in column (0-6)")
    print("  remove <row> <col> - Remove piece at (row, col)")
    print("  quit              - Exit game")
    print()

    while not state.is_terminal:
        legal_moves = get_legal_moves(state)
        if not legal_moves:
            print("No legal moves available.")
            break

        if state.current_player == 0:
            # Human's turn
            while True:
                try:
                    command = input("Your move: ").strip().split()
                    if not command:
                        continue
                    if command[0] == "quit":
                        return
                    elif command[0] == "place":
                        col = int(command[1])
                        from .moves import PlacementMove
                        move = PlacementMove(column=col)
                        if move in legal_moves:
                            state = apply_move(state, move)
                            break
                        else:
                            print("Illegal move. Try again.")
                    elif command[0] == "remove":
                        row, col = int(command[1]), int(command[2])
                        from .moves import RemovalMove
                        move = RemovalMove(row=row, col=col)
                        if move in legal_moves:
                            state = apply_move(state, move)
                            break
                        else:
                            print("Illegal move. Try again.")
                    else:
                        print("Unknown command.")
                except (ValueError, IndexError):
                    print("Invalid format. Try: place <col> or remove <row> <col>")
        else:
            # Bot's turn
            move = bot.choose_move(state)
            if move is None:
                print("Bot has no legal moves.")
                break
            print(f"\nBot plays: {move}")
            state = apply_move(state, move)

        print("\n" + state.display())

    # Game over
    print("\n" + "=" * 50)
    if state.winner is not None:
        print(f"Game over! Player {state.winner} wins!")
    else:
        print("Game over! Draw (board full).")
    print("=" * 50)


def watch_game(bot1, bot2, bot1_name: str, bot2_name: str):
    """
    Watch two bots play a game.

    Args:
        bot1: First bot.
        bot2: Second bot.
        bot1_name: Display name for bot1.
        bot2_name: Display name for bot2.
    """
    print(f"\n{bot1_name} vs {bot2_name}")
    print("=" * 50)

    state = GameState()
    move_count = 0

    while not state.is_terminal:
        legal_moves = get_legal_moves(state)
        if not legal_moves:
            break

        if state.current_player == 0:
            move = bot1.choose_move(state)
            bot_name = bot1_name
        else:
            move = bot2.choose_move(state)
            bot_name = bot2_name

        if move is None:
            break

        print(f"Move {move_count + 1}: {bot_name} plays {move}")
        state = apply_move(state, move)
        move_count += 1

    print("\n" + state.display())
    print("\n" + "=" * 50)
    if state.winner is not None:
        winner_name = bot1_name if state.winner == 0 else bot2_name
        print(f"Game over! {winner_name} (player {state.winner}) wins in {move_count} moves!")
    else:
        print(f"Game over! Draw in {move_count} moves.")
    print("=" * 50)


def main():
    """
    Simple CLI menu.
    """
    while True:
        print("\n" + "=" * 50)
        print("Connect 4 with Removals - AI Heuristic Design")
        print("=" * 50)
        print("1. Play against MinimaxBot (depth 5)")
        print("2. Watch two MinimaxBots (depth 5) play")
        print("3. Compare two heuristics (experiment)")
        print("4. Quit")
        print()

        choice = input("Choose option (1-4): ").strip()

        if choice == "1":
            play_interactive_game(depth=5)
        elif choice == "2":
            bot1 = MinimaxBot(evaluator=BasicEvaluator(), depth=5)
            bot2 = MinimaxBot(evaluator=BasicEvaluator(), depth=5)
            watch_game(bot1, bot2, "MinimaxBot-1 (depth 5)", "MinimaxBot-2 (depth 5)")
        elif choice == "3":
            run_experiment_interactive()
        elif choice == "4":
            print("Goodbye!")
            break
        else:
            print("Invalid choice.")


def run_experiment_interactive():
    """
    Interactive experiment runner.
    Compares BasicEvaluator (baseline) vs StudentEvaluator (with threat detection).
    """
    print("\n" + "=" * 50)
    print("Experiment: Compare Two Heuristics")
    print("=" * 50)
    print("Bot 1: MinimaxBot(depth=3, BasicEvaluator) - BASELINE")
    print("Bot 2: MinimaxBot(depth=3, StudentEvaluator) - WITH THREAT DETECTION")
    print()

    num_games = input("How many games? (default 10): ").strip()
    try:
        num_games = int(num_games) if num_games else 10
    except ValueError:
        num_games = 10

    print(f"\nRunning {num_games} games with alternating starts...")
    print("(Same search depth, different heuristics)\n")

    bot1 = MinimaxBot(evaluator=BasicEvaluator(), depth=4)
    bot2 = MinimaxBot(evaluator=StudentEvaluator(), depth=4)

    experiment = Experiment(
        bot1=bot1,
        bot2=bot2,
        num_games=num_games,
        verbose=True,
        seed=42,
        alternate_starts=True,
    )

    results = experiment.run()
    print(results)

    print()
    print("INTERPRETATION:")
    if results.bot2_wins > results.bot1_wins:
        print(f"  ✓ StudentEvaluator (threat detection) is stronger!")
        print(f"    It won {results.bot2_wins} vs {results.bot1_wins} games")
    elif results.bot1_wins > results.bot2_wins:
        print(f"  ✗ BasicEvaluator won more games ({results.bot1_wins} vs {results.bot2_wins})")
        print(f"    Threat detection may need tuning")
    else:
        print(f"  ~ They tied ({results.bot1_wins} wins each)")



if __name__ == "__main__":
    main()
