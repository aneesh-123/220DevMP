"""
CLI for Connect 4 with Removals.

Provides:
- Interactive play against a bot (user picks evaluator)
- Watch two bots play each other
- Batch experiments comparing bots
- Dynamic evaluator discovery from the evaluators/ folder
"""

from typing import Dict, List, Tuple, Optional

from .board import GameState
from .bots import MinimaxBot
from .evaluators import discover_evaluators
from .experiment import Experiment
from .rules import get_legal_moves, apply_move


DEFAULT_DEPTH = 4


def select_evaluator(
    evaluators: Dict[str, object], prompt: str = "Select an evaluator"
) -> Optional[Tuple[str, object]]:
    """Show available evaluators and let user select one."""
    eval_list = list(evaluators.items())
    if not eval_list:
        print("No evaluators available.")
        return None

    print(f"\n{prompt}:")
    for i, (name, _) in enumerate(eval_list, 1):
        print(f"  {i}. {name}")
    print(f"  0. Cancel")

    try:
        choice = int(input("\nEnter choice: ").strip())
        if choice == 0:
            return None
        if 1 <= choice <= len(eval_list):
            return eval_list[choice - 1]
        print("Invalid choice.")
        return None
    except ValueError:
        print("Invalid input.")
        return None


def select_two_evaluators(
    evaluators: Dict[str, object], prompt: str = "Select two evaluators"
) -> Optional[Tuple[str, object, str, object]]:
    """Let user select two evaluators (can be the same)."""
    eval_list = list(evaluators.items())
    if len(eval_list) < 1:
        print("No evaluators available.")
        return None

    print(f"\n{prompt}:")
    for i, (name, _) in enumerate(eval_list, 1):
        print(f"  {i}. {name}")
    print(f"  0. Cancel")

    try:
        choice1 = int(input("\nSelect first evaluator: ").strip())
        if choice1 == 0:
            return None
        if not (1 <= choice1 <= len(eval_list)):
            print("Invalid choice.")
            return None

        choice2 = int(input("Select second evaluator: ").strip())
        if choice2 == 0:
            return None
        if not (1 <= choice2 <= len(eval_list)):
            print("Invalid choice.")
            return None

        name1, eval1 = eval_list[choice1 - 1]
        name2, eval2 = eval_list[choice2 - 1]
        return name1, eval1, name2, eval2
    except ValueError:
        print("Invalid input.")
        return None


def play_interactive_game(bot, bot_name: str):
    """
    Play an interactive game against a bot.

    The human is player 0 (O), the bot is player 1 (X).
    """
    state = GameState()

    print(f"\nYou are player O. Playing against {bot_name}.")
    print("Commands:")
    print("  place <column>    - Drop a piece in column (0-6)")
    print("  remove <row> <col> - Remove piece at (row, col)")
    print("  quit              - Exit game")
    print("\n" + state.display())
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
            print(f"\n{bot_name} plays: {move}")
            state = apply_move(state, move)

        print("\n" + state.display())

    # Game over
    print("\n" + "=" * 50)
    if state.winner is not None:
        winner_symbol = 'O' if state.winner == 0 else 'X'
        winner_label = "You" if state.winner == 0 else bot_name
        print(f"Game over! Player {winner_symbol} ({winner_label}) wins!")
    else:
        print("Game over! Draw (board full).")
    print("=" * 50)


def watch_game(bot1, bot2, bot1_name: str, bot2_name: str):
    """Watch two bots play a game."""
    print(f"\n{bot1_name} (O) vs {bot2_name} (X)")
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

        print(f"\nMove {move_count + 1}: {bot_name} plays {move}")
        state = apply_move(state, move)
        move_count += 1
        print("\n" + state.display())

    print("\n" + "=" * 50)
    if state.winner is not None:
        winner_symbol = 'O' if state.winner == 0 else 'X'
        winner_name = bot1_name if state.winner == 0 else bot2_name
        print(f"Game over! Player {winner_symbol} ({winner_name}) wins in {move_count} moves!")
    else:
        print(f"Game over! Draw in {move_count} moves.")
    print("=" * 50)


def run_experiment(bot1, bot2, bot1_name: str, bot2_name: str):
    """Run a batch experiment comparing two bots."""
    print("\n" + "=" * 50)
    print("Experiment: Compare Two Bots")
    print("=" * 50)
    print(f"Bot 1: {bot1_name}")
    print(f"Bot 2: {bot2_name}")
    print()

    num_games = input("How many games? (default 10): ").strip()
    try:
        num_games = int(num_games) if num_games else 10
    except ValueError:
        num_games = 10

    print(f"\nRunning {num_games} games with alternating starts...")
    print("(Same search depth, different heuristics)\n")

    experiment = Experiment(
        bot1=bot1,
        bot2=bot2,
        num_games=num_games,
        verbose=True,
        seed=42,
        alternate_starts=True,
        bot1_name=bot1_name,
        bot2_name=bot2_name,
    )

    results = experiment.run()
    print(results)

    print()
    print("INTERPRETATION:")
    if results.bot2_wins > results.bot1_wins:
        print(f"  {bot2_name} is stronger!")
        print(f"    It won {results.bot2_wins} vs {results.bot1_wins} games")
    elif results.bot1_wins > results.bot2_wins:
        print(f"  {bot1_name} won more games ({results.bot1_wins} vs {results.bot2_wins})")
    else:
        print(f"  They tied ({results.bot1_wins} wins each)")


def main():
    """Main CLI menu with dynamic evaluator loading."""
    evaluators = discover_evaluators()

    if not evaluators:
        print("No evaluators found in evaluators/ folder.")
        return

    while True:
        print("\n" + "=" * 50)
        print("Connect 4 with Removals - AI Heuristic Design")
        print("=" * 50)
        print("1. Play against a bot")
        print("2. Watch two bots play")
        print("3. Compare two bots (experiment)")
        print("4. Quit")
        print()

        choice = input("Choose option (1-4): ").strip()

        if choice == "1":
            result = select_evaluator(evaluators, "Select a bot to play against")
            if result:
                name, evaluator = result
                bot = MinimaxBot(evaluator=evaluator, depth=DEFAULT_DEPTH)
                play_interactive_game(bot, name)

        elif choice == "2":
            result = select_two_evaluators(evaluators, "Select two bots to watch")
            if result:
                name1, eval1, name2, eval2 = result
                bot1 = MinimaxBot(evaluator=eval1, depth=DEFAULT_DEPTH)
                bot2 = MinimaxBot(evaluator=eval2, depth=DEFAULT_DEPTH)
                watch_game(bot1, bot2, name1, name2)

        elif choice == "3":
            result = select_two_evaluators(evaluators, "Select two bots to compare")
            if result:
                name1, eval1, name2, eval2 = result
                bot1 = MinimaxBot(evaluator=eval1, depth=DEFAULT_DEPTH)
                bot2 = MinimaxBot(evaluator=eval2, depth=DEFAULT_DEPTH)
                run_experiment(bot1, bot2, name1, name2)

        elif choice == "4":
            print("Goodbye!")
            break
        else:
            print("Invalid choice.")


if __name__ == "__main__":
    main()
