# Connect 4 With Removals: Heuristic Design Project

## Your Goal

Your job is to build a stronger Connect 4 bot by designing a better evaluation function.

The search algorithm, game rules, and command-line interface are already implemented for you. You are **not** writing minimax or alpha-beta pruning. Instead, you will:

1. Research useful heuristics for Connect 4.
2. Adapt those heuristics to this version of the game, where each player has **one removal move**.
3. Implement reusable heuristic helper functions.
4. Combine those heuristics into an evaluator.
5. Create your own test evaluators and board states.
6. Run experiments, analyze the results, and improve your bot.

## Game Variant

This project uses a modified version of Connect 4:

- Players can place pieces as usual.
- Each player also has **one removal** available during the game.
- A strong evaluator should consider both normal Connect 4 strategy and the effect of the one-time removal mechanic.

That means your heuristics should not only look at patterns like threats, center control, and winning opportunities, but also at how removals change the value of a position.

## What You Should Edit

You should only work in these locations:

- `src/connect4/heuristics.py`
- `src/connect4/evaluators/`
- `src/connect4/test_evaluators/`
- `src/connect4/board_states/`

You should **not** modify any other files in the project.

In particular, do **not** change:

- `search.py`
- `rules.py`
- `cli.py`
- `board.py`
- `bots.py`
- `experiment.py`
- any other infrastructure files outside the four locations listed above

Your work for this project should stay within `heuristics.py`, the `evaluators` folder, the `test_evaluators` folder, and the `board_states` folder.

## Main Tasks

### 1. Add Heuristic Functions

In `src/connect4/heuristics.py`, write reusable helper functions that each evaluate one aspect of a board position.

Any heuristic used by any evaluator in this project should be implemented in `src/connect4/heuristics.py`.

An example heuristic is already provided for you as a reference. Use that example to follow the expected style: one helper function per heuristic, each returning a numerical score.

Examples of possible heuristics:

- terminal win/loss bonus
- center control
- immediate winning moves
- opponent threats
- streaks of length 2 or 3
- removal advantage
- positions that become strong or weak after a removal

Each heuristic should return a numerical score.

## 2. Build Your Evaluator

In `src/connect4/evaluators/`, combine your heuristic functions into evaluator files.

Important:

- Every new evaluator you create should be placed in its **own separate `.py` file** inside `src/connect4/evaluators/`.
- Do not put multiple evaluators in one file unless there is a specific reason to do so.
- The provided `empty.py` file is a baseline example and a starting point.

Your evaluator should:

- call the helper functions from `heuristics.py`
- combine them into one final score
- return higher scores for positions that are better for the given player

The provided `Empty` evaluator is only a baseline. You are expected to improve it or create additional evaluator files as you experiment.

## 3. Create Test Evaluators

In `src/connect4/test_evaluators/`, add your own evaluator files to use as opponents during experiments.

Important:

- Each `.py` file in this folder is automatically discovered.
- Each file must define an `EVALUATOR` object.
- That evaluator must have an `evaluate(state, player)` method.
- It should also have a `__repr__()` so it displays nicely in the CLI.

These test evaluators do not need to be perfect. Their purpose is to give you different kinds of opponents so you can test whether your heuristics actually help.

## 4. Create Board States

In `src/connect4/board_states/`, add your own `.json` files representing interesting starting positions.

These board states should help you test specific situations, such as:

- immediate win opportunities
- positions where a block is required
- positions where a removal changes the best move
- positions where one player has strong center control
- near-endgame states

The experiment runner automatically loads every `.json` board state in this folder.

## 5. Run Experiments and Iterate

Use the experiment mode to test your evaluator against the available test evaluators on all available board states.

Your goal is not just to write heuristics once. Your goal is to:

- test them
- see what works
- identify weaknesses
- revise your evaluator
- test again

You should expect to go through multiple versions before arriving at your final evaluator.

## How The Project Is Structured

You are given the following infrastructure:

- `src/connect4/search.py`: minimax with alpha-beta pruning
- `src/connect4/rules.py`: legal moves and game rules
- `src/connect4/cli.py`: command-line interface
- `src/connect4/experiment.py`: experiment runner

This means the main intellectual work is heuristic design and experimental evaluation.

## Running The Project

From the project root, run:

```powershell
python -m src.connect4.cli
```

You will see a menu with options to:

- play against a bot
- watch two bots play
- run experiments on a bot

Use the experiment option to compare your evaluator against your test evaluators across your board states.

## File Conventions

### Evaluators

Files in `src/connect4/evaluators/` and `src/connect4/test_evaluators/` are auto-discovered.

Each evaluator should be in its own separate file.

Each evaluator file should define:

```python
EVALUATOR = YourEvaluatorClass()
```

### Board States

Files in `src/connect4/board_states/` should be JSON files. Each file should contain a board configuration and can also include:

- `name`
- `current_player`
- `removals_remaining`

Only valid board states will load.

## What A Good Submission Looks Like

A strong submission will include:

- several distinct heuristic helper functions
- a final evaluator that combines them thoughtfully
- multiple custom test evaluators
- multiple custom board states
- evidence that you tested and improved your evaluator over time

## What You Should Be Able To Explain

By the end of the project, you should be able to explain:

- which heuristics you implemented
- why you thought each heuristic would help
- how the one-removal rule affected your design
- what kinds of test positions you created and why
- what changed between your early and final versions
- what evidence suggests your final evaluator is better than your baseline

## Submission Checklist

Before submitting, make sure you have:

- added heuristic helper functions in `src/connect4/heuristics.py`
- kept all heuristics in `src/connect4/heuristics.py`
- created or updated evaluator files in `src/connect4/evaluators/`
- added your own files to `src/connect4/test_evaluators/`
- added your own files to `src/connect4/board_states/`
- run experiments on your evaluator
- revised your evaluator based on results

## Summary

This is an AI design project, not a search implementation project.

You are being asked to design a better heuristic bot for Connect 4 with one removal per player, test it carefully, and improve it based on evidence.
