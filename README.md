# Connect 4 with Removals - AI Heuristic Design

## Overview

This project focuses on building a strong game-playing agent for a modified version of Connect 4.

In addition to standard gameplay, each player has a limited number of removal moves that can be used during the game. Removing a piece causes all pieces above it to fall due to gravity.

This fundamentally changes the strategy of the game:

- Positions that look strong may become weak after a removal
- Hidden threats can emerge due to board reshaping

Your task is to design an evaluator that can reason about these dynamics effectively.

## What You Are Given

You are **NOT** building the game engine.

The following are already implemented:

- Game rules
- Move generation
- Minimax search
- CLI interface

Your job is to build the strategy layer of the bot.

## Your Goal

Design and evaluate a strong Connect 4 agent by:

- Implementing heuristics
- Combining them into evaluators
- Testing them on meaningful board states
- Improving performance through experimentation

## Key Concepts

### Heuristic

A function that scores one aspect of a board position.

Examples:

- Center control
- Threat creation
- Blocking opponent wins
- Removal flexibility

### Evaluator

A function that combines multiple heuristics into a single score.

This score is used by minimax to decide moves.

### Minimax

A recursive decision-making algorithm that:

- Maximizes your score
- Minimizes opponent score

Your evaluator is used at the leaves of the search tree.

## What You Can Modify

You may edit:

### `heuristics.c` and `heuristics.h`

- Implement reusable heuristic functions
- Each heuristic should evaluate one feature

### `evaluators/`

- Combine heuristics into full evaluation functions
- You can create multiple evaluators for comparison

### `opponents/` (optional)

- Create custom bots for benchmarking

### `board_states/` (optional)

- Add JSON board states for targeted testing

## What You MUST NOT Modify

### `engine/`

- The autograder will run your code against a clean engine
- Any modifications here will break grading

## Building

You need a C compiler (GCC, Clang, or MinGW).

This repo does not include a Makefile - you must set up your own build workflow.

Example compile command:

```sh
gcc -Wall -Wextra -O2 -Wno-unused-parameter -o connect4game \
  engine/board.c engine/moves.c engine/rules.c engine/search.c \
  engine/bot.c engine/experiment.c engine/board_loader.c \
  engine/cJSON.c engine/cli.c \
  heuristics.c evaluators/*.c opponents/*.c -lm
```

## Running

```sh
./connect4game
```

On Windows:

```powershell
.\connect4game.exe
```

## Menu Options

- Play against a bot
- Watch two bots play
- Run experiments (recommended)

## How to Approach the Project

- Start simple (e.g., piece count, center control)
- Add more strategic heuristics
- Run experiments
- Compare performance
- Iterate

## Example Heuristic

```c
float piece_count(const GameState *state, int player) {
    int opponent = 1 - player;
    return (float)(gamestate_count_pieces(state, player)
                 - gamestate_count_pieces(state, opponent));
}
```

## Example Evaluator

```c
float my_evaluate(const GameState *state, int player) {
    float score = 0.0f;

    score += terminal_state_bonus(state, player);
    score += piece_count(state, player);

    return score;
}
```

## Grading

This project is graded in two parts:

### Part 1: Report

You must justify your design decisions using evidence.

Your report should include:

- Heuristic descriptions and intuition
- Ablation studies
- Board-state evaluations
- Performance comparisons
- Failure analysis and improvements

(See assignment document for full requirements)

### Part 2: Autograder

Only the following files are graded:

- `heuristics.c`
- `heuristics.h`
- `evaluators/evaluators.h`
- Your final evaluator file(s)

Important:

- Any helper logic must be inside these files
- Code in other files will be ignored

## Submission

Submit:

- Your repository
- Your report (PDF)
