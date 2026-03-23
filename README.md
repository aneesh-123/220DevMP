# Connect 4 With Removals

## What This Game Is

This project uses a modified version of Connect 4.

- Players normally take turns dropping pieces into columns.
- The goal is to make 4 in a row horizontally, vertically, or diagonally.
- In this version, each player also has **one removal move** they can use during the game.

Because of that extra rule, positions must be evaluated differently than in standard Connect 4.

## Goal

Build a stronger Connect 4 bot by designing a better evaluation function.

The game engine, rules, CLI, and minimax search are already implemented. Your job is to design heuristics, combine them into evaluators, test them, and improve them.

Each player gets **one removal move** in addition to normal piece placement, so your heuristics should account for that rule.

## Key Terms

- `heuristic`: a helper function that scores one aspect of a position, such as a win, a threat, center control, or removal advantage
- `evaluator`: the object that combines several heuristics into one final score for a board state

The minimax search uses your evaluator to decide which move is best.

## Only Edit These Files

You should only work in:

- `connect4/heuristics.py`
- `connect4/evaluators/`
- `connect4/test_evaluators/`
- `connect4/board_states/`

Do **not** modify any other files.

## What To Do

### 1. Add Heuristics

Implement reusable heuristic helper functions in `connect4/heuristics.py`.

- Any heuristic used by an evaluator should be implemented in `heuristics.py`.
- An example heuristic is already provided for reference.
- Each heuristic should return a numerical score.

### 2. Build Evaluators

Create evaluator files in `connect4/evaluators/`.

- Every evaluator should be in its own separate `.py` file.
- Evaluators should call the helper functions from `heuristics.py`.
- `empty.py` is the starting baseline.

### 3. Create Test Evaluators

Add opponent evaluators in `connect4/test_evaluators/`.

- Each `.py` file in this folder is automatically discovered.
- Each file must define an `EVALUATOR` object.

### 4. Create Board States

Add `.json` board states in `connect4/board_states/`.

Use these to test important situations such as wins, blocks, removal decisions, and endgame positions.

### 5. Test and Iterate

Run experiments, study the results, revise your heuristics, and test again.

## Run The Program

From the project root, run:

```powershell
python -m connect4.cli
```

If `python` does not work, try:

```powershell
py -m connect4.cli
```

## Submission Checklist

Before submitting, make sure you have:

- added heuristic helper functions in `connect4/heuristics.py`
- kept all heuristics in `connect4/heuristics.py`
- created evaluator files in `connect4/evaluators/`
- added test evaluators in `connect4/test_evaluators/`
- added board states in `connect4/board_states/`
- run experiments and improved your evaluator
