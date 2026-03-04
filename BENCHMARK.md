# Benchmark Guide — Modified Connect 4 AI

## Why Benchmark?

The three AI modes implement progressively smarter search strategies:

| Mode | Strategy | What it adds |
|------|----------|-------------|
| 0 | Baseline minimax | Exhaustive search — explores every possible move sequence |
| 1 | Alpha-beta + move ordering | Prunes branches that can't affect the outcome; searches best-looking moves first |
| 2 | Alpha-beta + move ordering + transposition table | Caches previously evaluated positions to avoid redundant work |

Benchmarking lets you **prove these optimizations actually reduce computation**.
By running the same board positions at the same depth across all three modes,
you can directly compare nodes expanded and wall-clock time.

## How to Compile

```bash
gcc -O2 -std=c11 -Wall -Wextra -pedantic engine.c ai.c bench.c main.c -o c4
```

## How to Run Benchmarks

**All boards, all modes:**
```bash
./c4 --bench boards -d 6
```

**Single board file:**
```bash
./c4 --bench boards/board2_midgame.txt -d 5
```

**Only specific modes (e.g., compare mode 0 vs mode 2):**
```bash
./c4 --bench boards -d 5 --modes 0,2
```

**Export results to CSV for graphing:**
```bash
./c4 --bench boards -d 6 --csv results.csv
```

## Board File Format

Each `.txt` file in `boards/` describes a board position. The format is:

```
. . . . . . .       <- row 5 (TOP of the board)
. . . . . . .
. . O X . . .
. X O X . . .
. O X O . . .
X X O O . . .       <- row 0 (BOTTOM of the board)
TURN X               <- whose turn it is
REM X=2 O=1          <- removals remaining for each player
```

Rules:
- 6 lines of 7 characters each (`.`, `X`, or `O`), spaces are ignored
- First board line = top row (row 5), last board line = bottom row (row 0)
- `TURN X` or `TURN O` — which player moves next
- `REM X=N O=M` — removals remaining (0–3 each)

Files must be named `board*.txt` to be auto-discovered by `--bench <directory>`.

## What the Metrics Mean

| Metric | Description |
|--------|-------------|
| `NODES` | Number of recursive search calls (game tree nodes expanded). **Lower = faster search.** |
| `TIME_MS` | Wall-clock milliseconds for the AI to choose one move |
| `TT_PROBES` | (Mode 2 only) Number of transposition table lookups attempted |
| `TT_HITS` | (Mode 2 only) Number of lookups that returned a cached result, saving a full subtree search |
| `tt_hit_rate` | `TT_HITS / TT_PROBES` — higher means more positions were reused from cache |
| `speedup` | `mode_0_nodes / this_mode_nodes` — how many times fewer nodes this mode explores vs. baseline |

## What You Should Observe

### Mode 0 vs Mode 1 (alpha-beta)
- Mode 1 should expand **significantly fewer nodes** than Mode 0
- Speedups of 5–30x are typical at depth 4–5
- Both modes choose the **same move** (they compute the same minimax value; alpha-beta just skips provably irrelevant branches)

### Mode 1 vs Mode 2 (transposition table)
- Mode 2 should expand **fewer nodes** than Mode 1, especially on denser boards
- The TT hit rate shows what fraction of positions were cache hits
- Speedup over Mode 1 is typically 1.5–3x (less dramatic than alpha-beta, but compounds at higher depths)

### Depth matters
- At low depth (d=2–3), differences are small because the tree is tiny
- At higher depth (d=5–7), differences become dramatic — Mode 0 may take seconds while Mode 2 finishes in milliseconds
- **Try d=5 or d=6** for compelling comparisons

### Example output (depth 5, board2_midgame.txt)
```
Mode               avg_ms    avg_nodes  tt_hit_rate      speedup
Mode 0            1262.0      1195383          N/A        1.00x
Mode 1              57.0        35434          N/A       33.74x
Mode 2              29.0        18544        44.7%       64.46x
```

## Sample Boards

| File | Description |
|------|-------------|
| `board1_early.txt` | Early game — only 4 pieces, many open columns |
| `board2_midgame.txt` | Mid-game — 8 pieces, both central columns contested |
| `board3_dense.txt` | Dense board — 16 pieces, limited removals, O to move |
| `board4_threat.txt` | Threat scenario — O has 3 in a column, X must respond |
| `board5_complex.txt` | Complex — 16 pieces with both diagonal and vertical threats |

## Creating Your Own Board Files

1. Set up a position you're interested in (e.g., a tricky midgame)
2. Create a `.txt` file following the format above
3. Name it `board<something>.txt` and place it in `boards/`
4. Run: `./c4 --bench boards/board<something>.txt -d 6`

## CSV Output

With `--csv results.csv`, the benchmark writes a CSV file with columns:
```
board,mode,mode_name,depth,move,time_ms,nodes,tt_probes,tt_hits
```

This can be imported into Excel, Google Sheets, or Python (pandas) for
graphing nodes and time across modes.
