# Changes Made to Project

## Import Path Fixed
**Issue**: Tests were failing with `ModuleNotFoundError: No module named 'src'`

**Fix**: Updated all test files to use relative imports:
- Changed from `from src.connect4.board import ...`
- To `from connect4.board import ...`
- All 5 test files updated (test_board.py, test_moves.py, test_removals.py, test_search.py, test_win_detection.py)

Tests now run correctly:
```bash
cd src
python -m pytest ../tests -v
# Result: 72 passed
```

## RandomBot Removed

**Changes Made:**

1. **Removed RandomBot class** from `bots.py`
   - Only MinimaxBot remains
   - Default depth changed to **5** (was 4)
   - Cleaner API: students focus on evaluators, not bot selection

2. **Updated `experiment.py`**
   - Now requires both `bot1` and `bot2` as mandatory parameters
   - Removed default `RandomBot(seed=seed)` fallback
   - Signature: `Experiment(bot1, bot2, num_games=10, ...)`

3. **Updated `cli.py`** - Simplified menu:
   - Old options: Random bot, Minimax depth 4, Minimax depth 6, Random vs Random
   - New options:
     - Option 1: Play against MinimaxBot (depth 5)
     - Option 2: Play against MinimaxBot (depth 7)
     - Option 3: Watch two MinimaxBots (depth 5) play
     - Option 4: Compare two heuristics (experiment runner)

4. **Updated `__init__.py`**
   - Removed `RandomBot` from imports and `__all__`

5. **Updated README.md**
   - Removed all RandomBot references
   - Updated experiment example to use two MinimaxBots
   - Updated suggested workflow to not mention "random bot"

6. **Updated tests**
   - Removed `test_random_bot_chooses_legal_move`
   - Removed entire `TestRandomBotReproducibility` class
   - Tests reduced from 74 to 72 (all passing)

## Benefits of These Changes

1. **Cleaner API**: One bot type = less confusion for students
2. **Focus on Heuristics**: Students spend time on evaluators, not bot configuration
3. **Default Depth 5**: More reasonable default search depth (was 4)
4. **Experiment Requirements**: Forces explicit bot specification - more intentional
5. **CLI is Simpler**: Fewer options, clearer purpose

## Example Usage

**Before** (with RandomBot):
```python
from connect4.bots import RandomBot, MinimaxBot
from connect4.experiment import Experiment

bot1 = MinimaxBot(depth=4, ...)
# bot2 defaulted to RandomBot if not specified
exp = Experiment(bot1=bot1, num_games=10)
```

**After** (no RandomBot):
```python
from connect4.bots import MinimaxBot
from connect4.experiment import Experiment

bot1 = MinimaxBot(evaluator=BasicEvaluator(), depth=5)
bot2 = MinimaxBot(evaluator=MyEvaluator(), depth=5)
exp = Experiment(bot1=bot1, bot2=bot2, num_games=10)
```

## Verification

All tests pass:
```
✓ 72 tests pass in 0.19s
✓ All imports work correctly
✓ End-to-end demo runs successfully
✓ Experiment runner works with both bots
```

## Files Modified

1. tests/test_board.py - Import updates
2. tests/test_moves.py - Import updates
3. tests/test_removals.py - Import updates
4. tests/test_search.py - Import updates + RandomBot test removal
5. tests/test_win_detection.py - Import updates
6. src/connect4/__init__.py - Removed RandomBot import
7. src/connect4/bots.py - Removed RandomBot class, updated docstring
8. src/connect4/experiment.py - Removed RandomBot import, required bot2 parameter
9. src/connect4/cli.py - Simplified menu, no RandomBot options
10. README.md - Updated examples and documentation

## Files Not Modified
- board.py, moves.py, rules.py, search.py, heuristics.py - No changes needed
- ARCHITECTURE.md, QUICKSTART.md, PROJECT_SUMMARY.md - Still accurate
