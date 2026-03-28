# Grading Rubric — Connect 4 with Removals

## 1. Evaluator Performance

The autograder will run your evaluator against a large set of test opponents across many board states.
Each matchup plays 2 games (you go first, then second). Your grade for this section is based on your
overall win rate.

The test opponents and board states used for grading are **not** the same as the ones provided to you.
You should create your own board states and opponents to prepare.

**Important**: Engine files must pass the integrity check. If any engine file has been
modified, you receive 0 for this section.

---

## 2. Heuristic Design

Your `heuristics.c` and evaluator files will be reviewed for quality and variety.

A good submission includes multiple distinct heuristics that each capture a real aspect of game strategy.
Each heuristic should be non-trivial and correctly implemented.

Examples of strong heuristics:
- **Center control**: pieces in middle columns score higher
- **Threat detection**: 3-in-a-row with an open end
- **Blocking**: penalize positions where opponent has threats
- **Removal advantage**: value having your removal available
- **Connected pieces**: reward 2-in-a-row and 3-in-a-row configurations

---

## 3. Report

Submit a **PDF**, maximum **2 pages**.

Your report should cover:
- What heuristics did you implement and why?
- What weights did you choose? How did you arrive at them?
- What did you try that didn't work? What did you learn?

**Report guidelines**:
- Be concise. Explain your reasoning, not just your code.
- The failed attempts section is important. Showing what you tried and abandoned demonstrates understanding.
- Do not paste code into the report. Reference function names instead.

---

## Rules

- You may edit: `heuristics.c`, `heuristics.h`, files in `evaluators/`, files in `opponents/`, and `board_states/`.
- You must **not** modify anything in `engine/`. The integrity check enforces this.
- Your evaluator must only analyze the current board state. It must **not** perform its own
  search (no calling `get_legal_moves`, `apply_move`, or implementing your own minimax/recursion).
  An automated check will scan your code for violations, and your code will be reviewed.

---

## What You Submit

Run `./submit.sh yourname` to package your work into a zip, then send:
1. `yourname.zip` (your heuristics and evaluator files)
2. Your report as a PDF

Your files will be dropped into a clean copy of the repo and compiled against the autograder.
Make sure your code compiles cleanly with `make` before submitting.

## What You Must NOT Modify

Everything in `engine/`. The integrity check will detect modifications.
