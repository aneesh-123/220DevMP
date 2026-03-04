/*
 * ============================================================
 *  Modified Connect 4 — Project Scaffold
 * ============================================================
 *
 * COMPILE
 *   gcc -O2 -std=c11 -Wall -Wextra -pedantic *.c -o connect4
 *
 * RUN (interactive game, Mode 0 only for now)
 *   ./connect4 --play -m 0 -d 4
 *
 * RUN (benchmark)
 *   ./connect4 --bench boards -d 5
 *   ./connect4 --bench boards/board1_early.txt -d 4
 *
 * FLAGS
 *   -m <0|1|2|3|4>   AI mode (only 0 works now; others print stub message)
 *   -d <depth>        Search depth (default: 4)
 *   --bench <path>    Benchmark mode: path to board file or directory
 *   --play            Interactive human-vs-AI mode
 *   --ai X|O          Which side the AI plays (default: O)
 *   -h / --help       Print usage
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "engine.h"
#include "eval.h"
#include "metrics.h"
#include "ai_mode0.h"
#include "ai_mode1.h"
#include "ai_mode2.h"
#include "ai_mode3.h"
#include "ai_mode4.h"

/* ---- Constants ---- */
#define MAX_BOARDS 64

/* ---- Board file loading ---- */

typedef struct {
    GameState state;
    char      name[128];
} BoardEntry;

/* Extract filename from a path. */
static const char *basename_of(const char *path) {
    const char *p = path;
    const char *last = path;
    while (*p) {
        if (*p == '/' || *p == '\\') last = p + 1;
        p++;
    }
    return last;
}

/* Load a single board file. Returns 0 on success. */
static int load_board(const char *filepath, BoardEntry *out) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", filepath);
        return -1;
    }

    const char *fname = basename_of(filepath);
    strncpy(out->name, fname, sizeof(out->name) - 1);
    out->name[sizeof(out->name) - 1] = '\0';

    GameState *s = &out->state;
    memset(s->board, EMPTY, sizeof(s->board));
    s->turn  = PLAYER_X;
    s->rem_x = MAX_REMOVALS;
    s->rem_o = MAX_REMOVALS;

    char line[256];
    int rows_read = 0;

    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\n' || line[0] == '\r') continue;

        if (strncmp(line, "TURN", 4) == 0) {
            char player = 0;
            if (sscanf(line, "TURN %c", &player) == 1) {
                if (player == 'X' || player == 'x') s->turn = PLAYER_X;
                else if (player == 'O' || player == 'o') s->turn = PLAYER_O;
            }
            continue;
        }
        if (strncmp(line, "REM", 3) == 0) {
            int rx = MAX_REMOVALS, ro = MAX_REMOVALS;
            if (sscanf(line, "REM X=%d O=%d", &rx, &ro) == 2) {
                s->rem_x = rx;
                s->rem_o = ro;
            }
            continue;
        }

        /* Board row: first line = top (row 5), last = bottom (row 0). */
        if (rows_read < ROWS) {
            int internal_row = ROWS - 1 - rows_read;
            int col = 0;
            for (int i = 0; line[i] && col < COLS; i++) {
                char ch = line[i];
                if (ch == '.' || ch == 'X' || ch == 'O') {
                    s->board[internal_row][col] = ch;
                    col++;
                }
            }
            if (col != COLS) {
                fprintf(stderr, "Error in '%s': row %d has %d cells (need %d)\n",
                        filepath, rows_read, col, COLS);
                fclose(f);
                return -1;
            }
            rows_read++;
        }
    }
    fclose(f);

    if (rows_read != ROWS) {
        fprintf(stderr, "Error in '%s': read %d rows (need %d)\n",
                filepath, rows_read, ROWS);
        return -1;
    }
    return 0;
}

/* Load all board*.txt files from a directory. Returns count or -1 on error. */
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

static int load_boards_from_dir(const char *dirpath, BoardEntry *out) {
    /* Strip trailing slashes. */
    char dir[512];
    strncpy(dir, dirpath, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = '\0';
    size_t dlen = strlen(dir);
    while (dlen > 0 && (dir[dlen-1] == '/' || dir[dlen-1] == '\\'))
        dir[--dlen] = '\0';

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\board*.txt", dir);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        snprintf(pattern, sizeof(pattern), "%s/board*.txt", dir);
        h = FindFirstFileA(pattern, &fd);
        if (h == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "Error: no board*.txt files in '%s'\n", dirpath);
            return -1;
        }
    }
    int count = 0;
    do {
        if (count >= MAX_BOARDS) break;
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, fd.cFileName);
        if (load_board(fullpath, &out[count]) == 0) count++;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return count > 0 ? count : -1;
}
#else
#include <dirent.h>

static int load_boards_from_dir(const char *dirpath, BoardEntry *out) {
    DIR *d = opendir(dirpath);
    if (!d) {
        fprintf(stderr, "Error: cannot open directory '%s'\n", dirpath);
        return -1;
    }
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "board", 5) != 0) continue;
        size_t len = strlen(ent->d_name);
        if (len < 9 || strcmp(ent->d_name + len - 4, ".txt") != 0) continue;
        if (count >= MAX_BOARDS) break;
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, ent->d_name);
        if (load_board(fullpath, &out[count]) == 0) count++;
    }
    closedir(d);
    return count > 0 ? count : -1;
}
#endif

/* ---- is_directory ---- */
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

/* ---- Dispatch: call the right AI mode ---- */
static Move dispatch_ai(const GameState *s, int mode, int depth, Metrics *m) {
    switch (mode) {
        case 0:  return ai_choose_move_mode0(s, depth, m);
        case 1:  return ai_choose_move_mode1(s, depth, m);
        case 2:  return ai_choose_move_mode2(s, depth, m);
        case 3:  return ai_choose_move_mode3(s, depth, m);
        case 4:  return ai_choose_move_mode4(s, depth, m);
        default:
            fprintf(stderr, "Error: unknown mode %d\n", mode);
            exit(1);
    }
}

/* ---- Benchmark runner ---- */
static void run_benchmark(const BoardEntry *boards, int num_boards,
                          int mode, int depth) {
    printf("============================================================\n");
    printf("  BENCHMARK — mode=%d, depth=%d, boards=%d\n", mode, depth, num_boards);
    printf("============================================================\n\n");

    double  total_time  = 0;
    uint64_t total_nodes = 0;
    int     valid_runs  = 0;

    for (int b = 0; b < num_boards; b++) {
        printf("--- Board: %s ---\n", boards[b].name);
        state_print(&boards[b].state);
        printf("  Turn: %c | Removals X=%d O=%d\n\n",
               boards[b].state.turn,
               boards[b].state.rem_x,
               boards[b].state.rem_o);

        Metrics m;
        metrics_reset(&m, depth);
        metrics_start_timer(&m);
        Move best = dispatch_ai(&boards[b].state, mode, depth, &m);
        metrics_stop_timer(&m);

        char movebuf[64];
        move_to_str(&best, movebuf, sizeof(movebuf));

        printf("BOARD %-24s MODE %d DEPTH %d MOVE %-22s "
               "TIME_MS %8.1f NODES %10llu\n",
               boards[b].name, mode, depth, movebuf,
               m.time_ms, (unsigned long long)m.nodes_visited);

        if (m.tt_probes > 0) {
            printf("  TT_PROBES %llu TT_HITS %llu\n",
                   (unsigned long long)m.tt_probes,
                   (unsigned long long)m.tt_hits);
        }
        printf("\n");

        total_time  += m.time_ms;
        total_nodes += m.nodes_visited;
        valid_runs++;
    }

    printf("============================================================\n");
    printf("  SUMMARY (mode=%d, depth=%d, %d board(s))\n", mode, depth, valid_runs);
    printf("============================================================\n");
    if (valid_runs > 0) {
        printf("  MODE %d: avg_time_ms=%.1f, avg_nodes=%.0f\n",
               mode, total_time / valid_runs,
               (double)total_nodes / valid_runs);
    }
    printf("\n");
}

/* ---- Interactive play ---- */
static void run_interactive(int mode, int depth, char ai_player) {
    GameState state;
    state_init(&state);

    char human = (ai_player == PLAYER_X) ? PLAYER_O : PLAYER_X;

    printf("=== Modified Connect 4 ===\n");
    printf("Mode: %d | Depth: %d\n", mode, depth);
    printf("You are: %c | AI is: %c\n", human, ai_player);
    printf("Commands: d <col> (drop) | r <col> <row> (remove)\n\n");

    while (1) {
        state_print(&state);

        char winner = check_winner(&state);
        if (winner != EMPTY) {
            printf("*** %c wins! ***\n", winner);
            break;
        }
        if (is_board_full(&state)) {
            printf("*** Draw! ***\n");
            break;
        }

        printf("Turn: %c | Removals X=%d O=%d\n",
               state.turn, state.rem_x, state.rem_o);

        if (state.turn == ai_player) {
            printf("AI thinking (mode=%d, depth=%d)...\n", mode, depth);

            Metrics m;
            metrics_reset(&m, depth);
            metrics_start_timer(&m);
            Move best = dispatch_ai(&state, mode, depth, &m);
            metrics_stop_timer(&m);

            char movebuf[64];
            move_to_str(&best, movebuf, sizeof(movebuf));

            if (best.type == MOVE_DROP)
                apply_drop(&state, best.col);
            else
                apply_remove(&state, best.col, best.row);

            printf("AI move: %s\n", movebuf);
            printf("  time_ms=%.1f, nodes=%llu, depth=%d/%d\n\n",
                   m.time_ms, (unsigned long long)m.nodes_visited,
                   m.depth_completed, m.depth_requested);
        } else {
            int valid = 0;
            while (!valid) {
                printf("Enter move (d <col> | r <col> <row>): ");
                fflush(stdout);

                char line[256];
                if (!fgets(line, sizeof(line), stdin)) {
                    printf("\nGoodbye!\n");
                    return;
                }

                char cmd;
                int col, row;
                int parsed = sscanf(line, " %c %d %d", &cmd, &col, &row);

                if (parsed >= 2 && (cmd == 'd' || cmd == 'D')) {
                    if (is_legal_drop(&state, col)) {
                        apply_drop(&state, col);
                        valid = 1;
                    } else {
                        printf("Illegal drop (col=%d).\n", col);
                    }
                } else if (parsed >= 3 && (cmd == 'r' || cmd == 'R')) {
                    if (is_legal_remove(&state, col, row)) {
                        apply_remove(&state, col, row);
                        valid = 1;
                    } else {
                        printf("Illegal remove (col=%d, row=%d).\n", col, row);
                    }
                } else {
                    printf("Invalid input. Use: d <col>  or  r <col> <row>\n");
                }
            }
        }

        /* Check for win/draw immediately after the move. */
        char w = check_winner(&state);
        if (w != EMPTY) {
            state_print(&state);
            printf("*** %c wins! ***\n", w);
            break;
        }
        if (is_board_full(&state)) {
            state_print(&state);
            printf("*** Draw! ***\n");
            break;
        }

        switch_turn(&state);
    }
}

/* ---- Usage ---- */
static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s --play -m <0-4> -d <depth> [--ai X|O]\n", prog);
    printf("  %s --bench <path> -m <0-4> -d <depth>\n", prog);
    printf("\nFlags:\n");
    printf("  -m       AI mode (0=minimax, 1-4=stubs for now)\n");
    printf("  -d       Search depth (default: 4)\n");
    printf("  --play   Interactive human vs AI\n");
    printf("  --bench  Benchmark: path to board file or directory\n");
    printf("  --ai     Which side AI plays (default: O)\n");
}

/* ---- main ---- */
int main(int argc, char *argv[]) {
    int  mode       = 0;
    int  depth      = 4;
    char ai_player  = PLAYER_O;
    const char *bench_path = NULL;
    int  do_play    = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
            if (mode < 0 || mode > 4) {
                fprintf(stderr, "Error: mode must be 0..4\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            depth = atoi(argv[++i]);
            if (depth < 1 || depth > 20) {
                fprintf(stderr, "Error: depth must be 1..20\n");
                return 1;
            }
        } else if (strcmp(argv[i], "--ai") == 0 && i + 1 < argc) {
            i++;
            if (argv[i][0] == 'X' || argv[i][0] == 'x') ai_player = PLAYER_X;
            else if (argv[i][0] == 'O' || argv[i][0] == 'o') ai_player = PLAYER_O;
            else { fprintf(stderr, "Error: --ai must be X or O\n"); return 1; }
        } else if (strcmp(argv[i], "--bench") == 0 && i + 1 < argc) {
            bench_path = argv[++i];
        } else if (strcmp(argv[i], "--play") == 0) {
            do_play = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Benchmark mode */
    if (bench_path) {
        BoardEntry boards[MAX_BOARDS];
        int num_boards;
        if (is_directory(bench_path))
            num_boards = load_boards_from_dir(bench_path, boards);
        else
            num_boards = (load_board(bench_path, &boards[0]) == 0) ? 1 : -1;

        if (num_boards <= 0) {
            fprintf(stderr, "Error: failed to load boards from '%s'\n", bench_path);
            return 1;
        }
        run_benchmark(boards, num_boards, mode, depth);
        return 0;
    }

    /* Interactive mode */
    if (do_play) {
        run_interactive(mode, depth, ai_player);
        return 0;
    }

    /* No action specified */
    print_usage(argv[0]);
    return 0;
}
