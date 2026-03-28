# Connect 4 with Removals — AI Heuristic Design

## What This Game Is

A modified Connect 4 where each player also has **one removal move** per game.
Players take turns dropping pieces into columns to make 4 in a row (horizontal, vertical, or diagonal).
The removal adds a strategic twist: you can pull any piece off the board (gravity applies to pieces above).

## Your Goal

Build a stronger Connect 4 bot by designing a better evaluation function.
The game engine, rules, and minimax search are already implemented.
Your job is to write heuristics that score board positions well.

## Building

You need a C compiler (GCC, MinGW, or Clang).

```bash
# Default (gcc)
make

# Use a different compiler
make CC=clang
make CC=x86_64-w64-mingw32-gcc
```

## Running

```bash
./connect4game        # Linux/Mac
connect4game.exe      # Windows
```

Or use:
```bash
make run
```

## Menu Options

1. **Play against a bot** — select one of your evaluators and play interactively
2. **Watch two bots play** — pick any two bots (your evaluators + opponents)
3. **Run experiment** — benchmark an evaluator against all opponents

## What To Edit

You may edit: `heuristics.c`, `heuristics.h`, files in `evaluators/`, files in `opponents/`, and `board_states/`.

### 1. Add Heuristics — `heuristics.c` and `heuristics.h`

Write reusable functions that score one aspect of a position.
Each heuristic takes `(const GameState *state, int player)` and returns a `float`.

An example (`terminal_state_bonus`) is already provided.

### 2. Build Evaluators — `evaluators/`

Each evaluator is a separate `.c` file that combines your heuristics into a score.
Copy `empty.c` as a starting point.

To add a new evaluator:
1. Create a new `.c` file in `evaluators/` with an evaluate function
2. Open `evaluators/evaluators.h`
3. Add an `extern` declaration for your function
4. Add an entry to the `evaluator_bots[]` array
5. Rebuild with `make`

### 3. Create Test Opponents — `opponents/`

Add `.c` files with opponent evaluators to test your bot against. A weak opponent (`weak.c`)
is provided as a starting point.

To add a new opponent:
1. Create a new `.c` file in `opponents/` with an evaluate function
2. Open `opponents/opponents.h`
3. Add an `extern` declaration for your function
4. Add an entry to the `opponent_bots[]` array
5. Rebuild with `make`

### 4. Create Board States — `board_states/`

Add `.json` files to test specific situations. Format:

```json
{
  "name": "My Test Position",
  "current_player": 0,
  "removals_remaining": [1, 1],
  "board": [
    [null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null],
    [null, null, 0,    1,    null, null, null]
  ]
}
```

- Board is 6 rows x 7 columns. `null` = empty, `0` = player O, `1` = player X.
- Row 0 is the top, row 5 is the bottom.
- Pieces must obey gravity (no floating pieces).

### 5. Test and Iterate

Run experiments (option 3), study results, revise heuristics, repeat.

## Do NOT Modify

Everything in `engine/` is locked.
An integrity check runs at startup — if engine files are modified, experiments are blocked.

## Quick Start Example

1. Open `heuristics.c`, add a new function:
   ```c
   float center_control(const GameState *state, int player) {
       float score = 0.0f;
       int r, c;
       for (r = 0; r < ROWS; r++) {
           for (c = 0; c < COLS; c++) {
               if (state->board[r][c] == player) {
                   int dist = c - 3;
                   if (dist < 0) dist = -dist;
                   score += (float)(3 - dist);
               }
           }
       }
       return score;
   }
   ```
2. Declare it in `heuristics.h`
3. Create `evaluators/my_bot.c`:
   ```c
   #include "../heuristics.h"
   float my_evaluate(const GameState *state, int player) {
       float score = 0.0f;
       score += terminal_state_bonus(state, player);
       score += 2.0f * center_control(state, player);
       return score;
   }
   ```
4. Register in `evaluators/evaluators.h`:
   ```c
   extern float my_evaluate(const GameState *state, int player);
   // ... in the array:
   { my_evaluate, DEFAULT_DEPTH, "MyBot" },
   ```
5. `make && ./connect4game` — select MyBot from the menu

## Grading

See `GRADING.md` for the full rubric.

## Key Terms

- **Heuristic**: a function that scores one aspect of a position
- **Evaluator**: combines multiple heuristics into one score
- **Minimax**: the search algorithm that uses your evaluator to pick moves (depth = 4)
