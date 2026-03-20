# Project Architecture & Student Work Guide

## Overview

This is a complete, working Connect 4 game engine with an intentionally skeletal heuristics system for students to implement.

## Architecture Summary

### Provided Infrastructure (Do Not Modify)

#### 1. **moves.py** - Move Abstractions
- `PlacementMove(column)`: Drop a piece in a column
- `RemovalMove(row, col)`: Remove a piece from the board
- Immutable dataclasses for use with search algorithms

#### 2. **board.py** - Game State Management
- `GameState`: Complete board representation
  - `board`: 6×7 grid (None = empty, 0 = player 0, 1 = player 1)
  - `current_player`: Whose turn it is
  - `removals_remaining`: [player0_removals, player1_removals]
  - `is_terminal`, `winner`: Game status
  - `move_history`: For debugging/replay
- Utility methods: `copy()`, `get_cell()`, `set_cell()`, `count_pieces()`, etc.

#### 3. **rules.py** - Game Logic
- `get_legal_moves(state)`: Generate all legal moves
- `apply_move(state, move)`: Apply a move, handle gravity, detect wins
- Win detection: All directions (horizontal, vertical, both diagonals)
- Gravity logic: Pieces above a gap fall downward after removal
- Terminal state detection

#### 4. **search.py** - Minimax + Alpha-Beta Pruning
- `minimax(...)`: The core search algorithm
- `search(state, depth, evaluator)`: High-level search interface
- `SearchResult`: Move + score result
- **Key parameter**: `evaluator` - passed from the bot, hook for student heuristics

#### 5. **bots.py** - Bot Implementations
- `RandomBot`: Picks random legal moves (baseline)
- `MinimaxBot(evaluator, depth)`: Uses search with your evaluation function
- Clean interface for plugging in heuristics

#### 6. **experiment.py** - Experiment Runner
- `Experiment`: Run N games between two bots
- `ExperimentResults`: Statistics (wins, losses, draws)
- Handles alternating starting player for fair comparison
- Optional random seeding for reproducibility

#### 7. **cli.py** - Simple Runner
- Interactive play against bots
- Watch bot vs bot games
- Run batch experiments
- Simple menu interface

#### 8. **tests/** - Comprehensive Test Suite
- 74 tests covering:
  - Board representation and state management
  - Move generation and application
  - Removal gravity mechanics
  - Win detection (all directions)
  - Search correctness
  - Bot decision-making

---

## 🎯 Student Work: Heuristics

### Location: `src/connect4/heuristics.py`

This is the file where students focus their work.

### Current State

**BasicEvaluator**: A minimal baseline
```python
class BasicEvaluator:
    def evaluate(self, state, player) -> float:
        # Very simple heuristics:
        # - Terminal win/loss (large bonus/penalty)
        # - Piece count difference
        # - Tiny center control bonus
```

**StudentEvaluator**: Empty skeleton for students to fill

### What Students Should Implement

1. **Replace or extend `BasicEvaluator`** with stronger heuristics
2. **Implement suggested helper functions**:
   - `count_threats(state, player, opponent)`: How many ways can opponent win?
   - `count_winning_chances(state, player)`: How many ways can you win?
   - `evaluate_piece_clustering(state, player)`: Are your pieces grouped?
   - `evaluate_column_strength(state, player)`: Which columns do you dominate?
   - `evaluate_removal_value(state, player)`: How important are remaining removals?

3. **Create multiple evaluators** and test them against each other

4. **Combine multiple features** with weighted scoring

### Example Feature Ideas

- **Piece count**: Simple but effective
- **Threat detection**: Opponent one move away from winning = high penalty
- **Opportunity**: You can win in one move = high bonus
- **Center control**: Center columns are more versatile
- **Connectivity**: Your pieces in clusters = potential for 4-in-a-row
- **Removal tactics**: Is removing a piece more valuable than placing?
- **Positional value**: Different squares have different strategic value
- **Endgame patterns**: Special scoring for late-game positions

### Test & Experiment

```python
from connect4.bots import MinimaxBot
from connect4.heuristics import BasicEvaluator, StudentEvaluator
from connect4.experiment import Experiment

# Compare two evaluators
bot1 = MinimaxBot(evaluator=BasicEvaluator(), depth=5)
bot2 = MinimaxBot(evaluator=StudentEvaluator(), depth=5)

exp = Experiment(bot1=bot1, bot2=bot2, num_games=20, seed=42)
results = exp.run()
print(results)
```

---

## Running the Code

### Start from Ground Zero

```bash
cd src
python -m connect4.cli
```

Choose option 1-7 to play, watch bots, or run experiments.

### Run Tests

```bash
cd src
python -m pytest ../tests -v
```

### Simple Script

```python
from connect4.board import GameState
from connect4.bots import MinimaxBot, RandomBot
from connect4.heuristics import BasicEvaluator

state = GameState()

# Create bots
minimax = MinimaxBot(evaluator=BasicEvaluator(), depth=4)
random_bot = RandomBot()

# Play a game loop
while not state.is_terminal:
    if state.current_player == 0:
        move = minimax.choose_move(state)
    else:
        move = random_bot.choose_move(state)

    from connect4.rules import apply_move
    state = apply_move(state, move)

print(state.display())
print(f"Winner: {state.winner}")
```

---

## Key Design Decisions

1. **Immutable moves**: PlacementMove and RemovalMove are frozen (hashable)
2. **State copying**: GameState.copy() is deep, safe for minimax
3. **Side-effect-free rules**: apply_move returns new state
4. **Hook-based search**: Evaluator is a dependency injected into bots
5. **No hidden heuristics**: Search is pure minimax; all intelligence is in evaluation
6. **Simple, readable code**: Designed for educational understanding

---

## Testing Your Heuristics

1. **Unit tests**: Run the test suite to ensure you didn't break game rules
2. **Stats-based testing**: Use Experiment to run 20-100 games
3. **Qualitative testing**: Watch games (option 4 in CLI) to see bot behavior
4. **Comparative testing**: Compare multiple evaluators head-to-head
5. **Depth sensitivity**: Test same heuristic at different search depths

---

## Common Pitfalls to Avoid

- **Asymmetric scoring**: Make sure `evaluate(state, player)` scores FROM player's perspective
- **Unbalanced features**: Weights matter; a feature worth 1000 dominates others worth 1-10
- **Forgetting terminal states**: Always check if state is terminal first
- **Off-by-one errors**: Rows: 0=top, 5=bottom. Columns: 0-6 left to right
- **Mutable state**: Always use state.copy() in experiments if you modify state

---

## Educational Value

This project teaches:
- **Heuristic design**: What features matter in a game?
- **Experimmentation**: How to test and compare strategies scientifically
- **Search algorithms**: How minimax works (provided, but visible)
- **Evaluation function design**: The art of translating domain knowledge into numbers
- **Iterative improvement**: Hypothesis → implement → test → analyze → refine

The search algorithm is provided so students can focus on the hardest part: designing evaluation functions that guide intelligent play.

---

## File Dependency Graph

```
heuristics.py (STUDENT WORK)
    ↓
bots.py (MinimaxBot uses evaluator)
    ↓
search.py (uses evaluator to score positions)
    ↓
rules.py (applies moves, detects wins)
    ↓
board.py (manages game state)
    ↓
moves.py (defines move types)

experiment.py (runs games between bots)
cli.py (user interface)
tests/ (validates everything)
```

---

Start with `heuristics.py` and make it stronger!
