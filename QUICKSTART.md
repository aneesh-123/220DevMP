# Quick Start Guide

## 5-Minute Setup

```bash
cd src
python -m connect4.cli
```

Choose an option from the menu:
1. Play against Random bot
2. Play against Minimax (depth 4)
3. Play against Minimax (depth 6)
4. Watch Random vs Random
5. Watch Minimax vs Random
6. Run experiment
7. Quit

## Understand the Game in 5 Minutes

```python
from connect4.board import GameState
from connect4.moves import PlacementMove, RemovalMove
from connect4.rules import apply_move, get_legal_moves

state = GameState()
print(state.display())

# Get legal moves
moves = get_legal_moves(state)
print(f"Legal moves: {len(moves)}")

# Make a move
move = PlacementMove(column=3)
state = apply_move(state, move)

# Try a removal
if len(moves) > 7:  # If removal moves available
    removal = RemovalMove(row=5, col=0)
    state = apply_move(state, removal)

print(state.display())
```

## Implement Your First Evaluator (30 Minutes)

### Step 1: Read `heuristics.py`

Look at `BasicEvaluator.evaluate()` - it's simple!

```python
class BasicEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        # Already wins/loses? Return huge score
        if state.terminal:
            if state.winner == player:
                return 10000
            elif state.winner is not None:
                return -10000
            else:
                return 0

        # Otherwise: simple piece count
        my_pieces = state.count_pieces(player)
        opp_pieces = state.count_pieces(1 - player)

        return (my_pieces - opp_pieces) * 10
```

### Step 2: Create `MyEvaluator`

Copy and modify `StudentEvaluator` in `heuristics.py`:

```python
class MyEvaluator:
    def evaluate(self, state: GameState, player: int) -> float:
        if state.is_terminal:
            if state.winner == player:
                return 10000
            elif state.winner is not None:
                return -10000
            else:
                return 0

        # Your ideas here!
        # Example 1: Piece count
        score = (state.count_pieces(player) - state.count_pieces(1-player)) * 10

        # Example 2: Center control (pieces in columns 2-4 are better)
        center_bonus = 0
        for col in [2, 3, 4]:
            for row in range(6):
                if state.get_cell(row, col) == player:
                    center_bonus += 5

        return score + center_bonus
```

### Step 3: Test It

```python
from connect4.bots import MinimaxBot
from connect4.heuristics import BasicEvaluator, MyEvaluator
from connect4.experiment import Experiment

bot1 = MinimaxBot(evaluator=MyEvaluator(), depth=4)
bot2 = MinimaxBot(evaluator=BasicEvaluator(), depth=4)

exp = Experiment(bot1=bot1, bot2=bot2, num_games=10)
results = exp.run()
print(results)
```

## Test Ideas to Try

### 1. Piece Count Heuristic
```python
my_count = state.count_pieces(player)
opp_count = state.count_pieces(1 - player)
return (my_count - opp_count) * 50
```
**Expected**: Slightly better than random

### 2. Threat Detection
```python
opponent = 1 - player

# Count opponent's "almost wins" (3-in-a-row)
threat_count = 0
for row in range(state.ROWS):
    for col in range(state.COLS - 2):
        cols_with_opp = sum(1 for i in range(3)
                            if state.get_cell(row, col+i) == opponent)
        if cols_with_opp >= 2:  # 2 opponent pieces, might threaten
            threat_count += 1

return -threat_count * 100
```
**Expected**: Better at blocking opponent

### 3. Center Control
```python
center_value = 0
for col in [2, 3, 4]:  # Center columns
    for row in range(state.ROWS):
        if state.get_cell(row, col) == player:
            center_value += 10

return center_value
```
**Expected**: Focuses on strategic squares

### 4. Combination
```python
# Piece count
piece_score = (state.count_pieces(player) - state.count_pieces(1-player)) * 10

# Center control
center_score = 0
for col in [2, 3, 4]:
    for row in range(state.ROWS):
        if state.get_cell(row, col) == player:
            center_score += 5

# Threat defense
opponent = 1 - player
threat_count = ...  # (implement as above)
threat_score = -threat_count * 50

return piece_score + center_score + threat_score
```
**Expected**: Best so far

## Running Tests

```bash
cd src
python -m pytest ../tests -v
```

Should see: `74 passed in 0.17s`

## Key Files to Know

| File | Purpose | Student Touch? |
|------|---------|---|
| `heuristics.py` | Evaluation functions | ✅ YES - Main work |
| `moves.py` | Move definitions | ❌ No |
| `board.py` | Game state | ❌ No |
| `rules.py` | Game logic (apply move, win detection) | ❌ No |
| `search.py` | Minimax + alpha-beta | ❌ No |
| `bots.py` | Bot interfaces | ❌ No |
| `experiment.py` | Batch game runner | Maybe (advanced) |
| `cli.py` | User interface | Maybe (advanced) |

## Common Questions

**Q: How deep should I search?**
A: Start with depth=4 or 5. Deeper = slower but stronger. Experiment!

**Q: Why isn't my evaluator making the bot stronger?**
A: Maybe the weight is wrong (too high/low)? Or the feature isn't predictive? Test piece-by-piece.

**Q: Can I modify the search?**
A: Not recommended for this project. Focus on evaluation.

**Q: How do I know if my heuristic is good?**
A: Run 20+ games against a baseline and track win rate.

**Q: What's the difference between search depth and heuristic quality?**
A: Depth 4 + weak heuristic beats depth 2 + strong heuristic. Both matter!

## Next Steps

1. ✅ Run the CLI and play a game
2. ✅ Read `heuristics.py` and understand `BasicEvaluator`
3. ✅ Implement your first custom evaluator (piece count idea)
4. ✅ Run 10 games: Your evaluator vs `BasicEvaluator`, both depth=4
5. ✅ Analyze results - did your heuristic help?
6. ✅ Try a new idea (center control, threat detection, etc.)
7. ✅ Iterate and improve

Good luck! 🚀
