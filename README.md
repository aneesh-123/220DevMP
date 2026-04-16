# Extra Credit Assignment - Connect 4 With Removal

## Introduction

The goals of this MP are:
- Get yourself comfortable with using AI-assisted coding to solve a task  
- Design a strong game-playing agent for a modified version of Connect 4  
- Evaluate and improve upon your design via experimentation  
- Prove with evidence why your design is superior  

---

## Rules of the Game

This project uses a modified version of Connect 4.

- Players alternate turns.  
- On a normal turn, a player drops a piece into one of the columns.  
- A player wins by making four of their pieces in a row horizontally, vertically, or diagonally.  
- In addition to normal placement moves, each player has **ONE removal move** during the game.  
- A removal move allows a player to remove a piece from the board according to the rules implemented by the engine.  
- After a piece is removed, gravity applies, and all pieces above that location fall downward.  
- Once a winning board state is created, the game ends.  

Because removals can drastically change board structure, a strong evaluator must reason about both current threats and future board reshaping. Because this rule changes the structure of the game, board positions must be evaluated differently than in standard Connect 4.

In particular:
- A position that looks strong in the short term may become weak if an important piece can be removed  
- A position that appears safe may contain hidden threats enabled by future removals  

The skeleton code, along with a weak evaluator, provides an option to play this modified version to see for yourself. Instructions on how to run this are below.

---

## Key Terms

### Heuristic
A helper function that scores one feature of a board position.  
Examples include:
- Rewarding center control  
- Detecting immediate wins  
- Penalizing opponent threats  
- Valuing removal flexibility  

### Evaluator
A function that combines several heuristics into one final numerical score for a board state.

### Board State
A saved game position used to test whether an evaluator behaves sensibly in important situations.

### Minimax
The minimax algorithm is a decision-making strategy used to choose the best move in a two-player game by simulating future gameplay.

- It recursively explores possible moves  
- You (the maximizing player) try to maximize the score  
- Your opponent (the minimizing player) tries to minimize it  
- At the deepest level, your evaluator assigns a score to each board state  
- These scores are propagated back up to determine the optimal move  

We provide you with a strong version of the minimax algorithm already implemented that you should **NOT modify**. It is highly recommended to understand it thoroughly.

**Resources:**
- Minimax overview: https://www.youtube.com/watch?v=SLgZhpDsrfc  
- Heuristics + Alpha-Beta: https://www.youtube.com/watch?v=DV5d31z1xTI  

---

## Heuristics

It is unrealistic for an evaluator to fully explore the game tree in real time due to CPU constraints.

- Minimax explores up to depth = 5  
- Leaf nodes rely heavily on heuristic evaluation  

A heuristic estimates how good a board position is without searching to the end of the game.

A good heuristic:
- Reflects real strategic value  
- Rewards strong positions  
- Aligns higher scores with higher win probability  

Your job:
- Research heuristics  
- Implement them  
- Combine them into a strong evaluator  

---

## Your Task

You are **NOT** building the full game engine.

The following are already provided:
- Game engine  
- Rules  
- Move generation  
- Minimax framework  
- Runner for experiments  

Your job is to implement the **strategy layer**.

### You must edit:

#### `heuristics.c` and `heuristics.h`
- Implement reusable heuristic functions  
- Each heuristic should score one aspect of the board  

#### `evaluators/final_evaluator.c` and `evaluators/evaluators.h`
- Combine heuristics into a final score  
- You may create multiple evaluators to compare ideas  
- You can copy and modify the starter evaluator  

#### `opponents/`
- Create custom opponent bots  
- Useful for benchmarking (weak → strong → removal-aware)  

#### `board_states/`
- Create JSON board states  
- Represent key tactical scenarios:
  - Immediate wins  
  - Blocks  
  - Forks  
  - Traps  
  - Removal-based positions  

---

## Important Restrictions

**DO NOT MODIFY anything in `engine/`**

The autograder will only extract:
- `heuristics.c`
- `heuristics.h`
- `evaluators/evaluators.h`
- `evaluators/final_evaluator.c`

Any code outside these files will be ignored.

## Build and Run (in EWS)
We have provided you with a Makefile to easily compile the code. Below is how you would compile and run the connect4 program.

make clean
make
./connect4game


---
## AI Use

Unlike other MPs, this one is designed to observe and test how well you use AI to solve complex problems. Therefore, the use of AI is permitted and encouraged. However, keep in mind how and why you are using it because the report will ask you questions related to it.

---

## Grading

This MP will be graded in two parts:

### Report

The report can be found here: **LINK TO THE REPORT REQUIREMENTS**

This report will be essential to proving why your evaluator is satisfactory.

---

### Autograder

The autograder will only extract and evaluate the following files from your submission:

- `heuristics.c`
- `heuristics.h`
- `evaluators/evaluators.h`
- `evaluators/final_evaluator.c`

No other files will be used during grading.

Any logic, helper functions, or dependencies that your evaluator relies on must be defined within these files. Code placed in other files will be ignored by the autograder.

---

### Evaluation Methods

#### 1. Scorer

Your evaluator will be tested on a set of predefined board states. For each board state, it must select a recommended move. The autograder will compare your chosen move against a labeled set of:
- Weak  
- OK  
- Strong  

to assess its quality.

---

#### 2. Test Opponents

Your evaluator will also be evaluated by playing against a diverse set of opponents across many board states. Its overall performance in these games will be used to determine its strength.

