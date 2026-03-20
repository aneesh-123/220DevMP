# Connect 4 with Removals: AI Heuristic Design Project

## Overview

This is a modified Connect 4 game designed as an educational project focused on **heuristic evaluation function design and experimentation**.

The project provides:
- ✅ A complete, working game engine
- ✅ Move generation and legality checking
- ✅ Minimax with alpha-beta pruning search
- ✅ Bot infrastructure and runner framework
- ❌ **Sophisticated heuristics** (you implement these!)

The goal is for students to focus on **designing, implementing, and testing evaluation functions** rather than building game mechanics or search algorithms from scratch.

## Modified Rules

Standard Connect 4 with an added twist: **Removal Moves**

### Basic Rules
- 6 rows × 7 columns board
- Players alternate turns (player 0 and player 1)
- Normal move: drop a piece into a non-full column
- Win condition: 4 in a row (horizontal, vertical, or diagonal)
- Game ends immediately upon win

### The Removal Rule (Modified)
- Each player has **exactly 2 removal moves per game**
- On your turn, instead of placing a piece, you may **remove any occupied piece** from the board
- After removal, gravity applies: pieces above the gap fall downward
- Removals count as your turn and decrement your removal count
- A removal can win instantly if it creates a 4-in-a-row (rare edge case)

## Project Structure

```
.
├── README.md
├── requirements.txt
├── src/
│   └── connect4/
│       ├── __init__.py
│       ├── board.py           # Board state and display
│       ├── moves.py           # Move types (Placement, Removal)
│       ├── rules.py           # Game logic, move legality, win detection
│       ├── search.py          # Minimax with alpha-beta pruning
│       ├── bots.py            # Bot interfaces and implementations
│       ├── heuristics.py       # ⭐ STUDENT WORK: Evaluation functions
│       ├── experiment.py       # Batch game runner for experiments
│       └── cli.py             # Simple runner scripts
└── tests/
    ├── test_board.py
    ├── test_moves.py
    ├── test_removals.py
    ├── test_win_detection.py
    └── test_search.py
```

## What's Provided

### Infrastructure (You Don't Need to Touch)
- **`board.py`**: Board representation, state management
- **`moves.py`**: Move abstractions (PlacementMove, RemovalMove)
- **`rules.py`**: All game rules, move legality, win detection, gravity logic
- **`search.py`**: Minimax + alpha-beta pruning implementation
- **`bots.py`**: MinimaxBot interface
- **`experiment.py`**: Framework for running experiments and collecting statistics
- **`cli.py`**: Simple runner for common scenarios
- **`tests/`**: Comprehensive test suite

### Your Work (Heuristics)
- **`heuristics.py`**: This is where you implement evaluation functions!

## Where to Implement Heuristics

The `heuristics.py` file contains skeleton classes and functions with TODO markers.

### Current Baseline
A minimal `BasicEvaluator` exists to make the code run, but it is intentionally weak:
- Detects wins (very high score) and losses (very low score)
- Minimal piece count difference
- No strategic features

### Your Task
Replace or extend the evaluation function with smarter heuristics. Examples of features you might explore:
- Piece count and distribution
- Threat detection (opponent's one-move wins)
- Defensive opportunities (blocking opponent threats)
- Center control
- Cluster formation
- Removal move value and fragility
- Endgame patterns
- And more!

See `heuristics.py` for placeholder function skeletons and detailed comments.

## Running the Code

### Installation
```bash
cd src
# No external dependencies required!
```

### Playing Bot vs Bot

```python
from connect4.cli import main
main()

# Or from command line:
python -m connect4.cli
```

### Running Experiments

```python
from connect4.experiment import Experiment
from connect4.bots import MinimaxBot
from connect4.heuristics import BasicEvaluator

# Run 10 games: MinimaxBot with BasicEvaluator vs another evaluator
bot1 = MinimaxBot(evaluator=BasicEvaluator(), depth=5)
bot2 = MinimaxBot(evaluator=BasicEvaluator(), depth=5)

experiment = Experiment(
    bot1=bot1,
    bot2=bot2,
    num_games=10,
    verbose=True,
    seed=42
)
results = experiment.run()
print(results)
```

### Running Tests

```bash
cd src
python -m pytest ../tests -v
```

## Key Classes and Interfaces

### Move Types
```python
from connect4.moves import PlacementMove, RemovalMove

move = PlacementMove(column=3)
move = RemovalMove(row=2, col=4)
```

### Game State
```python
from connect4.board import GameState

state = GameState()
print(state.current_player)       # 0 or 1
print(state.removals_remaining)   # [3, 3]
print(state.is_terminal)          # bool
print(state.winner)               # None, 0, or 1
```

### Bots
```python
from connect4.bots import MinimaxBot
from connect4.heuristics import BasicEvaluator

# Minimax bot with your evaluator
evaluator = BasicEvaluator()
bot = MinimaxBot(evaluator=evaluator, depth=5)

# Get a move
move = bot.choose_move(state)
```

### Evaluator Interface
```python
class MyEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        """
        Return a score for the given state from player's perspective.
        Positive = good for player, Negative = bad for player.
        """
        # Your heuristic logic here
        return score
```

## Suggested Workflow

1. **Understand the game**: Read the rules and run the CLI
2. **Study the provided code**: Explore `search.py`, `rules.py`, `board.py`
3. **Read `heuristics.py`**: See the skeleton and TODOs
4. **Implement a simple evaluator**: Start with one or two features
5. **Run experiments**: Compare different evaluators at different depths
6. **Iterate**: Add features, test, analyze win rates and game patterns
7. **Analyze results**: Which features help? Which hurt? When?

## Tips

- Start with simple heuristics (piece count, center control)
- Use `Experiment` to run many games and see patterns
- Think about what features matter in the endgame vs. midgame
- Consider how removals change strategy
- Write multiple evaluators and compare them
- Use fixed seeds (`seed=42`) for reproducible experiments

## Educational Note

This project is designed so that **search and game mechanics are provided**, allowing you to focus entirely on the most interesting part: **designing evaluation functions that guide a strong AI**.

The quality of your heuristics determines how well your bot plays. A deeper search with a weak evaluator will still lose to shallow search with great heuristics.

Good luck! 🚀
