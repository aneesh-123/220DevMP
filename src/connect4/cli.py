"""
Simple CLI and runner for Connect 4.

Provides:
- Interactive play against Bot A or Bot B
- Watch bots play each other
- Batch experiments comparing bots
- Simple command-line interface
"""

import importlib
import importlib.util
import sys
from pathlib import Path
from typing import Dict

from .board import GameState
from .bots import MinimaxBot
from .evaluators import BasicEvaluator, StudentEvaluator
from .experiment import Experiment
from .rules import get_legal_moves, apply_move


# Create the two bots globally
BOT_A = MinimaxBot(evaluator=BasicEvaluator(), depth=4)
BOT_B = MinimaxBot(evaluator=StudentEvaluator(), depth=4)

BOT_A_NAME = "Bot A (BasicEvaluator)"
BOT_B_NAME = "Bot B (Threat Detection)"


def discover_bots() -> Dict[str, MinimaxBot]:
    """
    Dynamically load all bots from the student_bots/ folder.

    Returns a dictionary of {bot_name: bot_object} where bot_name is obtained
    from the evaluator's __repr__() method.
    """
    bots = {}
    bot_dir = Path(__file__).parent / "student_bots"

    # Only proceed if student_bots directory exists
    if not bot_dir.exists():
        return bots

    # Ensure the project root is in sys.path for imports
    project_root = Path(__file__).parent.parent.parent
    if str(project_root) not in sys.path:
        sys.path.insert(0, str(project_root))

    # Scan all .py files in the student_bots directory
    for bot_file in sorted(bot_dir.glob("*.py")):
        if bot_file.name.startswith("_"):
            continue

        module_name = bot_file.stem
        try:
            spec = importlib.util.spec_from_file_location(
                f"connect4.student_bots.{module_name}",
                bot_file,
            )
            if spec and spec.loader:
                module = importlib.util.module_from_spec(spec)
                sys.modules[spec.name] = module
                spec.loader.exec_module(module)

                # Extract the BOT object
                if hasattr(module, "BOT"):
                    bot = module.BOT
                    # Get the bot name from the evaluator's __repr__()
                    bot_name = repr(bot.evaluator)
                    bots[bot_name] = bot
        except (ImportError, AttributeError, Exception) as e:
            print(f"Warning: Could not load bot from {bot_file.name}: {e}")

    return bots


def play_interactive_game(bot, bot_name: str):
    """
    Play an interactive game against a bot.

    The human is player 0 (O), the bot is player 1 (X).

    Args:
        bot: The bot to play against.
        bot_name: Display name for the bot.
    """
    state = GameState()

    print("\n" + state.display())
    print(f"\nYou are player 0 (O). Playing against {bot_name}.")
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
            print(f"\n{bot_name} plays: {move}")
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

        print(f"\nMove {move_count + 1}: {bot_name} plays {move}")
        state = apply_move(state, move)
        move_count += 1
        print("\n" + state.display())

    print("\n" + "=" * 50)
    if state.winner is not None:
        winner_name = bot1_name if state.winner == 0 else bot2_name
        print(f"Game over! {winner_name} wins in {move_count} moves!")
    else:
        print(f"Game over! Draw in {move_count} moves.")
    print("=" * 50)


def select_bot(available_bots: Dict[str, MinimaxBot], prompt: str = "Select a bot") -> tuple:
    """
    Show available bots and let user select one.

    Args:
        available_bots: Dictionary of bot_name -> bot_object
        prompt: Message to display to user

    Returns:
        Tuple of (bot_name, bot_object) or (None, None) if user cancels
    """
    bot_list = list(available_bots.items())
    if not bot_list:
        print("No bots available.")
        return None, None

    print(f"\n{prompt}:")
    for i, (bot_name, _) in enumerate(bot_list, 1):
        print(f"  {i}. {bot_name}")
    print(f"  0. Cancel")

    try:
        choice = int(input("\nEnter choice: ").strip())
        if choice == 0:
            return None, None
        if 1 <= choice <= len(bot_list):
            bot_name, bot = bot_list[choice - 1]
            return bot_name, bot
        print("Invalid choice.")
        return None, None
    except ValueError:
        print("Invalid input.")
        return None, None


def select_two_bots(
    available_bots: Dict[str, MinimaxBot], prompt: str = "Select two different bots"
) -> tuple:
    """
    Let user select two different bots.

    Args:
        available_bots: Dictionary of bot_name -> bot_object
        prompt: Message to display to user

    Returns:
        Tuple of (bot1_name, bot1_object, bot2_name, bot2_object) or None if error
    """
    bot_list = list(available_bots.items())
    if len(bot_list) < 2:
        print("Need at least 2 bots available.")
        return None

    print(f"\n{prompt}:")
    for i, (bot_name, _) in enumerate(bot_list, 1):
        print(f"  {i}. {bot_name}")
    print(f"  0. Cancel")

    try:
        choice1 = int(input("\nSelect first bot: ").strip())
        if choice1 == 0:
            return None
        if not (1 <= choice1 <= len(bot_list)):
            print("Invalid choice.")
            return None

        choice2 = int(input("Select second bot (must be different): ").strip())
        if choice2 == 0:
            return None
        if not (1 <= choice2 <= len(bot_list)):
            print("Invalid choice.")
            return None

        if choice1 == choice2:
            print("Please select two different bots.")
            return None

        bot1_name, bot1 = bot_list[choice1 - 1]
        bot2_name, bot2 = bot_list[choice2 - 1]
        return bot1_name, bot1, bot2_name, bot2
    except ValueError:
        print("Invalid input.")
        return None


def play_interactive_with_selection(available_bots: Dict[str, MinimaxBot]):
    """
    Let user select a bot and play interactively.

    Args:
        available_bots: Dictionary of bot_name -> bot_object
    """
    if not available_bots:
        print("No bots available.")
        return

    bot_name, bot = select_bot(available_bots, "Select a bot to play against")
    if bot is None:
        return

    play_interactive_game(bot, bot_name)


def watch_with_selection(available_bots: Dict[str, MinimaxBot]):
    """
    Let user select two bots and watch them play.

    Args:
        available_bots: Dictionary of bot_name -> bot_object
    """
    if len(available_bots) < 2:
        print("Need at least 2 bots available.")
        return

    result = select_two_bots(available_bots, "Select two bots to watch")
    if result is None:
        return

    bot1_name, bot1, bot2_name, bot2 = result
    watch_game(bot1, bot2, bot1_name, bot2_name)


def run_experiment_with_selection(available_bots: Dict[str, MinimaxBot]):
    """
    Let user select two bots and run an experiment comparing them.

    Args:
        available_bots: Dictionary of bot_name -> bot_object
    """
    if len(available_bots) < 2:
        print("Need at least 2 bots available.")
        return

    result = select_two_bots(available_bots, "Select two bots to compare")
    if result is None:
        return

    bot1_name, bot1, bot2_name, bot2 = result

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
    )

    results = experiment.run()
    print(results)

    print()
    print("INTERPRETATION:")
    if results.bot2_wins > results.bot1_wins:
        print(f"  ✓ {bot2_name} is stronger!")
        print(f"    It won {results.bot2_wins} vs {results.bot1_wins} games")
    elif results.bot1_wins > results.bot2_wins:
        print(f"  ✗ {bot1_name} won more games ({results.bot1_wins} vs {results.bot2_wins})")
        print(f"    {bot2_name} may need tuning")
    else:
        print(f"  ~ They tied ({results.bot1_wins} wins each)")


def main():
    """
    Main CLI menu with dynamic bot loading.
    """
    # Load available bots
    available_bots = discover_bots()

    while True:
        print("\n" + "=" * 50)
        print("Connect 4 with Removals - AI Heuristic Design")
        print("=" * 50)
        print("1. Play against a bot")
        print("2. Watch Bots Play Against Each Other")
        print("3. Compare two bots (experiment)")
        print("4. Quit")
        print()

        choice = input("Choose option (1-4): ").strip()

        if choice == "1":
            play_interactive_with_selection(available_bots)
        elif choice == "2":
            watch_with_selection(available_bots)
        elif choice == "3":
            run_experiment_with_selection(available_bots)
        elif choice == "4":
            print("Goodbye!")
            break
        else:
            print("Invalid choice.")


if __name__ == "__main__":
    main()
