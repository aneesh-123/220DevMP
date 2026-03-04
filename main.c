/*
 * ============================================================
 *  Modified Connect 4 — Proof-of-Concept Prototype
 * ============================================================
 *
 * RULES
 *   Board: 7 columns x 6 rows. Row 0 = bottom, row 5 = top.
 *   Players: X (first) and O (second).
 *   Each turn, a player does ONE of:
 *     (A) DROP:   place a piece in the lowest empty cell of a column.
 *     (B) REMOVE: remove any occupied cell (costs 1 removal; max 3 per player).
 *                 After removal, pieces above fall down (gravity in that column).
 *   Win: 4 in a row (horizontal, vertical, or diagonal). Immediate.
 *   Draw: board full, no winner.
 *
 * COMPILE
 *   gcc -O2 -std=c11 -Wall -Wextra -pedantic engine.c ai.c bench.c main.c -o c4
 *
 * RUN (interactive game)
 *   ./c4 -m <0|1|2> -d <depth> [--ai X|O]
 *
 * RUN (benchmark mode)
 *   ./c4 --bench boards/ -d 6
 *   ./c4 --bench boards/ -d 6 --modes 0,1,2
 *   ./c4 --bench boards/ -d 5 --csv results.csv
 *   ./c4 --bench boards/board2_midgame.txt -d 4
 *
 * FLAGS
 *   -m 0|1|2   AI mode for interactive play (default: 1)
 *   -d N       Search depth (default: 4)
 *   --ai X|O   Which side the AI plays in interactive mode (default: O)
 *   --bench P  Benchmark mode: P is a directory of board*.txt files
 *              or a single board file. Runs benchmark and exits.
 *   --modes L  Comma-separated list of modes to benchmark (default: 0,1,2)
 *   --csv F    Write benchmark results to CSV file F
 *   -h         Print usage and exit
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "engine.h"
#include "ai.h"
#include "bench.h"

/* Cross-platform high-resolution timer.
   Returns current time in milliseconds (double). */
static double get_time_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6;
#endif
}

static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s -m <0|1|2> -d <depth> [--ai X|O]          (interactive)\n", prog);
    printf("  %s --bench <path> -d <depth> [--modes 0,1,2]  (benchmark)\n", prog);
    printf("\nFlags:\n");
    printf("  -m       AI mode: 0=minimax, 1=alpha-beta, 2=alpha-beta+TT\n");
    printf("  -d       Search depth (e.g. 3, 4, 5)\n");
    printf("  --ai     Which side AI plays (default: O)\n");
    printf("  --bench  Path to a board file or directory of board*.txt files\n");
    printf("  --modes  Comma-separated modes to benchmark (default: 0,1,2)\n");
    printf("  --csv    Write benchmark CSV to file\n");
}

/* Parse a comma-separated list of mode integers (e.g. "0,1,2").
   Returns the count of modes parsed. */
static int parse_modes(const char *str, int *modes_out, int max_modes) {
    int count = 0;
    const char *p = str;
    while (*p && count < max_modes) {
        int m = (int)strtol(p, NULL, 10);
        if (m >= 0 && m <= 2) {
            modes_out[count++] = m;
        }
        /* Advance past the number and any comma. */
        while (*p && *p != ',') p++;
        if (*p == ',') p++;
    }
    return count;
}

/* Check if path is a directory (works on both Windows/MinGW and POSIX).
   Handles trailing slashes which stat() may reject on Windows. */
static int is_directory(const char *path) {
    char clean[512];
    strncpy(clean, path, sizeof(clean) - 1);
    clean[sizeof(clean) - 1] = '\0';
    size_t len = strlen(clean);
    while (len > 1 && (clean[len-1] == '/' || clean[len-1] == '\\'))
        clean[--len] = '\0';

    struct stat sb;
    if (stat(clean, &sb) != 0) return 0;
    return S_ISDIR(sb.st_mode);
}

/* ---- Interactive game (unchanged logic) ---- */
static int run_interactive(int mode, int depth, char ai_player) {
    GameState state;
    init_state(&state);

    char human_player = (ai_player == PLAYER_X) ? PLAYER_O : PLAYER_X;

    static const char *mode_names[] = {
        "baseline minimax",
        "alpha-beta + move ordering",
        "alpha-beta + move ordering + transposition table"
    };

    printf("=== Modified Connect 4 ===\n");
    printf("AI mode: %d (%s)\n", mode, mode_names[mode]);
    printf("Search depth: %d\n", depth);
    printf("You are: %c   AI is: %c\n", human_player, ai_player);
    printf("Commands: d <col> (drop) | r <col> <row> (remove)\n");
    printf("Removals remaining — X: %d, O: %d\n\n",
           state.removals_left_X, state.removals_left_O);

    while (1) {
        print_board(&state);

        char winner = check_winner(&state);
        if (winner != EMPTY) {
            printf("*** %c wins! ***\n", winner);
            break;
        }
        if (is_draw(&state)) {
            printf("*** Draw! ***\n");
            break;
        }

        printf("Turn: %c | Removals left — X: %d, O: %d\n",
               state.current_player,
               state.removals_left_X, state.removals_left_O);

        if (state.current_player == ai_player) {
            printf("AI is thinking (mode=%d, depth=%d)...\n", mode, depth);

            AIStats stats;
            double t0 = get_time_ms();
            Move best = ai_choose_move(&state, mode, depth, &stats);
            double t1 = get_time_ms();
            double elapsed = t1 - t0;

            if (best.type == MOVE_DROP) {
                printf("AI move: DROP col=%d\n", best.col);
                apply_drop(&state, best.col);
            } else {
                printf("AI move: REMOVE col=%d row=%d\n", best.col, best.row);
                apply_remove(&state, best.col, best.row);
            }

            printf("  time_ms=%.1f, nodes=%lld, depth=%d\n",
                   elapsed, stats.nodes_expanded, depth);
            if (mode == 2) {
                printf("  tt_probes=%lld, tt_hits=%lld\n",
                       stats.tt_probes, stats.tt_hits);
            }
            printf("\n");

        } else {
            int valid = 0;
            while (!valid) {
                printf("Enter move (d <col> | r <col> <row>): ");
                fflush(stdout);

                char line[256];
                if (!fgets(line, sizeof(line), stdin)) {
                    printf("\nGoodbye!\n");
                    return 0;
                }

                char cmd;
                int col, row;
                int parsed = sscanf(line, " %c %d %d", &cmd, &col, &row);

                if (parsed >= 2 && (cmd == 'd' || cmd == 'D')) {
                    if (is_legal_drop(&state, col)) {
                        apply_drop(&state, col);
                        valid = 1;
                    } else {
                        printf("Illegal drop (col=%d). Try again.\n", col);
                    }
                } else if (parsed >= 3 && (cmd == 'r' || cmd == 'R')) {
                    if (is_legal_remove(&state, col, row)) {
                        apply_remove(&state, col, row);
                        valid = 1;
                    } else {
                        printf("Illegal remove (col=%d, row=%d). "
                               "Check: cell occupied? removals left (%d)?\n",
                               col, row, current_removals_left(&state));
                    }
                } else {
                    printf("Invalid input. Use: d <col>  or  r <col> <row>\n");
                }
            }
        }

        char winner2 = check_winner(&state);
        if (winner2 != EMPTY) {
            print_board(&state);
            printf("*** %c wins! ***\n", winner2);
            break;
        }
        if (is_draw(&state)) {
            print_board(&state);
            printf("*** Draw! ***\n");
            break;
        }

        switch_player(&state);
    }

    return 0;
}

/* ---- Main ---- */
int main(int argc, char *argv[]) {
    int mode = 1;
    int depth = 4;
    char ai_player = PLAYER_O;
    const char *bench_path = NULL;
    const char *csv_path = NULL;
    int bench_modes[3] = {0, 1, 2};
    int num_bench_modes = 3;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
            if (mode < 0 || mode > 2) {
                fprintf(stderr, "Error: mode must be 0, 1, or 2.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            depth = atoi(argv[++i]);
            if (depth < 1 || depth > 20) {
                fprintf(stderr, "Error: depth must be 1..20.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--ai") == 0 && i + 1 < argc) {
            i++;
            if (argv[i][0] == 'X' || argv[i][0] == 'x')
                ai_player = PLAYER_X;
            else if (argv[i][0] == 'O' || argv[i][0] == 'o')
                ai_player = PLAYER_O;
            else {
                fprintf(stderr, "Error: --ai must be X or O.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--bench") == 0 && i + 1 < argc) {
            bench_path = argv[++i];
        } else if (strcmp(argv[i], "--modes") == 0 && i + 1 < argc) {
            num_bench_modes = parse_modes(argv[++i], bench_modes, 3);
            if (num_bench_modes == 0) {
                fprintf(stderr, "Error: --modes needs at least one mode.\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--csv") == 0 && i + 1 < argc) {
            csv_path = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Initialize AI subsystem (Zobrist keys). */
    ai_init_zobrist();

    /* ---- Benchmark mode ---- */
    if (bench_path) {
        BoardEntry boards[MAX_BOARDS];
        int num_boards;

        if (is_directory(bench_path)) {
            num_boards = load_boards_from_dir(bench_path, boards);
        } else {
            /* Single file. */
            num_boards = (load_board(bench_path, &boards[0]) == 0) ? 1 : -1;
        }

        if (num_boards <= 0) {
            fprintf(stderr, "Error: failed to load boards from '%s'\n",
                    bench_path);
            return 1;
        }

        run_benchmark(boards, num_boards, depth,
                      bench_modes, num_bench_modes, csv_path);
        return 0;
    }

    /* ---- Interactive mode ---- */
    if (mode == 2) tt_clear();
    return run_interactive(mode, depth, ai_player);
}
