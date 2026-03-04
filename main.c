/*
 * ============================================================
 *  Modified Connect 4 — Project Scaffold
 * ============================================================
 *
 * COMPILE
 *   gcc -O2 -std=c11 -Wall -Wextra -pedantic *.c -o connect4
 *
 * RUN (interactive game)
 *   ./connect4 --play -m 0 -d 4
 *
 * RUN (benchmark single mode)
 *   ./connect4 --bench boards -m 0 -d 5
 *
 * RUN (compare ALL modes side by side)
 *   ./connect4 --bench-all boards -d 5
 *
 * FLAGS
 *   -m <0|1|2|3|4|5>  AI mode (default: 0)
 *   -d <depth>         Search depth (default: 4)
 *   --bench <path>     Benchmark one mode on board(s)
 *   --bench-all <path> Benchmark ALL modes (0-5) with comparison table
 *   --play             Interactive human-vs-AI mode
 *   --ai X|O           Which side the AI plays (default: O)
 *   -h / --help        Print usage
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
#include "ai_mode5.h"

/* ---- Constants ---- */
#define MAX_BOARDS 64
#define NUM_MODES  6

static const char *MODE_NAMES[NUM_MODES] = {
    "Minimax",
    "AlphaBeta",
    "AB+MoveOrder",
    "AB+MO+TT",
    "AB+MO+TT+ID",
    "GoldStandard"
};

/* ---- Board file loading ---- */

typedef struct {
    GameState state;
    char      name[128];
} BoardEntry;

static const char *basename_of(const char *path) {
    const char *p = path, *last = path;
    while (*p) { if (*p == '/' || *p == '\\') last = p + 1; p++; }
    return last;
}

static int load_board(const char *filepath, BoardEntry *out) {
    FILE *f = fopen(filepath, "r");
    if (!f) { fprintf(stderr, "Error: cannot open '%s'\n", filepath); return -1; }

    const char *fname = basename_of(filepath);
    strncpy(out->name, fname, sizeof(out->name) - 1);
    out->name[sizeof(out->name) - 1] = '\0';

    GameState *s = &out->state;
    memset(s->board, EMPTY, sizeof(s->board));
    s->turn = PLAYER_X; s->rem_x = MAX_REMOVALS; s->rem_o = MAX_REMOVALS;

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
            int rx, ro;
            if (sscanf(line, "REM X=%d O=%d", &rx, &ro) == 2) {
                s->rem_x = rx; s->rem_o = ro;
            }
            continue;
        }
        if (rows_read < ROWS) {
            int internal_row = ROWS - 1 - rows_read;
            int col = 0;
            for (int i = 0; line[i] && col < COLS; i++) {
                char ch = line[i];
                if (ch == '.' || ch == 'X' || ch == 'O')
                    s->board[internal_row][col++] = ch;
            }
            if (col != COLS) {
                fprintf(stderr, "Error in '%s': row %d has %d cells\n",
                        filepath, rows_read, col);
                fclose(f); return -1;
            }
            rows_read++;
        }
    }
    fclose(f);
    if (rows_read != ROWS) {
        fprintf(stderr, "Error in '%s': read %d rows\n", filepath, rows_read);
        return -1;
    }
    return 0;
}

/* ---- Directory listing ---- */
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static int load_boards_from_dir(const char *dirpath, BoardEntry *out) {
    char dir[512];
    strncpy(dir, dirpath, sizeof(dir) - 1); dir[sizeof(dir) - 1] = '\0';
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
            fprintf(stderr, "Error: no board*.txt in '%s'\n", dirpath);
            return -1;
        }
    }
    int count = 0;
    do {
        if (count >= MAX_BOARDS) break;
        char fp[512];
        snprintf(fp, sizeof(fp), "%s/%s", dir, fd.cFileName);
        if (load_board(fp, &out[count]) == 0) count++;
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return count > 0 ? count : -1;
}
#else
#include <dirent.h>
static int load_boards_from_dir(const char *dirpath, BoardEntry *out) {
    DIR *d = opendir(dirpath);
    if (!d) { fprintf(stderr, "Error: cannot open '%s'\n", dirpath); return -1; }
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "board", 5) != 0) continue;
        size_t len = strlen(ent->d_name);
        if (len < 9 || strcmp(ent->d_name + len - 4, ".txt") != 0) continue;
        if (count >= MAX_BOARDS) break;
        char fp[512];
        snprintf(fp, sizeof(fp), "%s/%s", dirpath, ent->d_name);
        if (load_board(fp, &out[count]) == 0) count++;
    }
    closedir(d);
    return count > 0 ? count : -1;
}
#endif

static int is_directory(const char *path) {
    char clean[512];
    strncpy(clean, path, sizeof(clean) - 1); clean[sizeof(clean) - 1] = '\0';
    size_t len = strlen(clean);
    while (len > 1 && (clean[len-1] == '/' || clean[len-1] == '\\'))
        clean[--len] = '\0';
    struct stat sb;
    if (stat(clean, &sb) != 0) return 0;
    return S_ISDIR(sb.st_mode);
}

static int load_boards(const char *path, BoardEntry *boards) {
    if (is_directory(path))
        return load_boards_from_dir(path, boards);
    return (load_board(path, &boards[0]) == 0) ? 1 : -1;
}

/* ---- Dispatch ---- */
static Move dispatch_ai(const GameState *s, int mode, int depth, Metrics *m) {
    /* Clear TT before modes 3/4/5 to ensure fair per-board comparison. */
    if (mode == 3 || mode == 4) mode3_tt_clear();
    if (mode == 5) mode5_tt_clear();

    switch (mode) {
        case 0:  return ai_choose_move_mode0(s, depth, m);
        case 1:  return ai_choose_move_mode1(s, depth, m);
        case 2:  return ai_choose_move_mode2(s, depth, m);
        case 3:  return ai_choose_move_mode3(s, depth, m);
        case 4:  return ai_choose_move_mode4(s, depth, m);
        case 5:  return ai_choose_move_mode5(s, depth, m);
        default: fprintf(stderr, "Error: unknown mode %d\n", mode); exit(1);
    }
}

/* ---- Single-mode benchmark ---- */
static void run_benchmark(const BoardEntry *boards, int num_boards,
                          int mode, int depth) {
    printf("============================================================\n");
    printf("  BENCHMARK — mode=%d (%s), depth=%d, boards=%d\n",
           mode, MODE_NAMES[mode], depth, num_boards);
    printf("============================================================\n\n");

    double total_time = 0;
    uint64_t total_nodes = 0;
    uint64_t total_ttp = 0, total_tth = 0;

    for (int b = 0; b < num_boards; b++) {
        printf("--- Board: %s ---\n", boards[b].name);
        state_print(&boards[b].state);
        printf("  Turn: %c | Removals X=%d O=%d\n\n",
               boards[b].state.turn, boards[b].state.rem_x, boards[b].state.rem_o);

        Metrics met;
        metrics_reset(&met, depth);
        metrics_start_timer(&met);
        Move best = dispatch_ai(&boards[b].state, mode, depth, &met);
        metrics_stop_timer(&met);

        char mb[64];
        move_to_str(&best, mb, sizeof(mb));

        printf("BOARD %-24s MODE %d DEPTH %d MOVE %-22s "
               "TIME_MS %8.1f NODES %10llu\n",
               boards[b].name, mode, depth, mb,
               met.time_ms, (unsigned long long)met.nodes_visited);
        if (met.tt_probes > 0)
            printf("  TT_PROBES %llu TT_HITS %llu\n",
                   (unsigned long long)met.tt_probes,
                   (unsigned long long)met.tt_hits);
        printf("\n");

        total_time  += met.time_ms;
        total_nodes += met.nodes_visited;
        total_ttp   += met.tt_probes;
        total_tth   += met.tt_hits;
    }

    printf("============================================================\n");
    printf("  SUMMARY (mode=%d, depth=%d, %d board(s))\n", mode, depth, num_boards);
    printf("============================================================\n");
    printf("  MODE %d: avg_time_ms=%.1f, avg_nodes=%.0f\n",
           mode, total_time / num_boards, (double)total_nodes / num_boards);
    if (total_ttp > 0)
        printf("  TT hit rate: %.1f%%\n", 100.0 * total_tth / total_ttp);
    printf("\n");
}

/* ---- All-modes benchmark with comparison table ---- */

/* Per-board, per-mode result for the comparison table. */
typedef struct {
    char     move_str[64];
    double   time_ms;
    uint64_t nodes;
    uint64_t tt_probes;
    uint64_t tt_hits;
} BenchResult;

static void run_bench_all(const BoardEntry *boards, int num_boards, int depth) {
    BenchResult results[MAX_BOARDS][NUM_MODES];

    printf("============================================================\n");
    printf("  BENCHMARK ALL MODES — depth=%d, boards=%d\n", depth, num_boards);
    printf("============================================================\n\n");

    /* Run each board through each mode. */
    for (int b = 0; b < num_boards; b++) {
        printf("--- Board: %s ---\n", boards[b].name);
        state_print(&boards[b].state);
        printf("  Turn: %c | Removals X=%d O=%d\n\n",
               boards[b].state.turn, boards[b].state.rem_x, boards[b].state.rem_o);

        for (int mode = 0; mode < NUM_MODES; mode++) {
            Metrics met;
            metrics_reset(&met, depth);
            metrics_start_timer(&met);
            Move best = dispatch_ai(&boards[b].state, mode, depth, &met);
            metrics_stop_timer(&met);

            BenchResult *r = &results[b][mode];
            move_to_str(&best, r->move_str, sizeof(r->move_str));
            r->time_ms   = met.time_ms;
            r->nodes     = met.nodes_visited;
            r->tt_probes = met.tt_probes;
            r->tt_hits   = met.tt_hits;

            printf("  Mode %d %-14s  MOVE %-22s  TIME %8.1f ms  NODES %10llu",
                   mode, MODE_NAMES[mode], r->move_str,
                   r->time_ms, (unsigned long long)r->nodes);
            if (r->tt_probes > 0)
                printf("  TT %.1f%%", 100.0 * r->tt_hits / r->tt_probes);
            printf("\n");
        }
        printf("\n");
    }

    /* ---- Per-board comparison table ---- */
    printf("============================================================\n");
    printf("  PER-BOARD COMPARISON (depth=%d)\n", depth);
    printf("============================================================\n");
    printf("  %-24s", "Board");
    for (int mode = 0; mode < NUM_MODES; mode++)
        printf(" %12s", MODE_NAMES[mode]);
    printf("\n");

    /* Separator line */
    printf("  ");
    for (int i = 0; i < 24 + NUM_MODES * 13; i++) printf("-");
    printf("\n");

    /* Nodes row per board */
    for (int b = 0; b < num_boards; b++) {
        printf("  %-24s", boards[b].name);
        for (int mode = 0; mode < NUM_MODES; mode++)
            printf(" %12llu", (unsigned long long)results[b][mode].nodes);
        printf("\n");
    }

    /* ---- Summary averages ---- */
    printf("\n");
    printf("============================================================\n");
    printf("  SUMMARY AVERAGES (depth=%d, %d board(s))\n", depth, num_boards);
    printf("============================================================\n");
    printf("  %-16s %10s %12s %10s %10s\n",
           "Mode", "avg_ms", "avg_nodes", "tt_hit%", "speedup");
    printf("  ");
    for (int i = 0; i < 60; i++) printf("-");
    printf("\n");

    double baseline_nodes = 0;

    for (int mode = 0; mode < NUM_MODES; mode++) {
        double   sum_time  = 0;
        uint64_t sum_nodes = 0;
        uint64_t sum_ttp   = 0, sum_tth = 0;

        for (int b = 0; b < num_boards; b++) {
            sum_time  += results[b][mode].time_ms;
            sum_nodes += results[b][mode].nodes;
            sum_ttp   += results[b][mode].tt_probes;
            sum_tth   += results[b][mode].tt_hits;
        }

        double avg_time  = sum_time / num_boards;
        double avg_nodes = (double)sum_nodes / num_boards;
        if (mode == 0) baseline_nodes = avg_nodes;

        char tt_buf[16] = "N/A";
        if (sum_ttp > 0)
            snprintf(tt_buf, sizeof(tt_buf), "%.1f%%",
                     100.0 * sum_tth / sum_ttp);

        char spd_buf[16];
        if (baseline_nodes > 0 && avg_nodes > 0)
            snprintf(spd_buf, sizeof(spd_buf), "%.1fx",
                     baseline_nodes / avg_nodes);
        else
            snprintf(spd_buf, sizeof(spd_buf), "1.0x");

        printf("  Mode %d %-10s %10.1f %12.0f %10s %10s\n",
               mode, MODE_NAMES[mode], avg_time, avg_nodes, tt_buf, spd_buf);
    }
    printf("\n");
}

/* ---- Interactive play ---- */
static void run_interactive(int mode, int depth, char ai_player) {
    GameState state;
    state_init(&state);
    char human = (ai_player == PLAYER_X) ? PLAYER_O : PLAYER_X;

    printf("=== Modified Connect 4 ===\n");
    printf("Mode: %d (%s) | Depth: %d\n", mode, MODE_NAMES[mode], depth);
    printf("You are: %c | AI is: %c\n", human, ai_player);
    printf("Commands: d <col> (drop) | r <col> <row> (remove)\n\n");

    while (1) {
        state_print(&state);
        char winner = check_winner(&state);
        if (winner != EMPTY) { printf("*** %c wins! ***\n", winner); break; }
        if (is_board_full(&state)) { printf("*** Draw! ***\n"); break; }

        printf("Turn: %c | Removals X=%d O=%d\n", state.turn, state.rem_x, state.rem_o);

        if (state.turn == ai_player) {
            printf("AI thinking (mode=%d, depth=%d)...\n", mode, depth);
            Metrics met;
            metrics_reset(&met, depth);
            metrics_start_timer(&met);
            Move best = dispatch_ai(&state, mode, depth, &met);
            metrics_stop_timer(&met);

            char mb[64];
            move_to_str(&best, mb, sizeof(mb));
            if (best.type == MOVE_DROP) apply_drop(&state, best.col);
            else apply_remove(&state, best.col, best.row);

            printf("AI move: %s\n", mb);
            printf("  time_ms=%.1f, nodes=%llu, depth=%d/%d",
                   met.time_ms, (unsigned long long)met.nodes_visited,
                   met.depth_completed, met.depth_requested);
            if (met.tt_probes > 0)
                printf(", tt_hits=%llu/%llu",
                       (unsigned long long)met.tt_hits,
                       (unsigned long long)met.tt_probes);
            printf("\n\n");
        } else {
            int valid = 0;
            while (!valid) {
                printf("Enter move (d <col> | r <col> <row>): ");
                fflush(stdout);
                char line[256];
                if (!fgets(line, sizeof(line), stdin)) { printf("\nGoodbye!\n"); return; }
                char cmd; int col, row;
                int parsed = sscanf(line, " %c %d %d", &cmd, &col, &row);
                if (parsed >= 2 && (cmd == 'd' || cmd == 'D')) {
                    if (is_legal_drop(&state, col)) { apply_drop(&state, col); valid = 1; }
                    else printf("Illegal drop (col=%d).\n", col);
                } else if (parsed >= 3 && (cmd == 'r' || cmd == 'R')) {
                    if (is_legal_remove(&state, col, row)) { apply_remove(&state, col, row); valid = 1; }
                    else printf("Illegal remove (col=%d, row=%d).\n", col, row);
                } else printf("Invalid input. Use: d <col>  or  r <col> <row>\n");
            }
        }

        char w = check_winner(&state);
        if (w != EMPTY) { state_print(&state); printf("*** %c wins! ***\n", w); break; }
        if (is_board_full(&state)) { state_print(&state); printf("*** Draw! ***\n"); break; }
        switch_turn(&state);
    }
}

/* ---- Usage ---- */
static void print_usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s --play -m <0-5> -d <depth> [--ai X|O]\n", prog);
    printf("  %s --bench <path> -m <0-5> -d <depth>\n", prog);
    printf("  %s --bench-all <path> -d <depth>\n", prog);
    printf("\nFlags:\n");
    printf("  -m          AI mode (0=minimax, 1=AB, 2=AB+MO, 3=AB+MO+TT, 4=+ID, 5=gold)\n");
    printf("  -d          Search depth (default: 4)\n");
    printf("  --play      Interactive human vs AI\n");
    printf("  --bench     Benchmark single mode on board(s)\n");
    printf("  --bench-all Benchmark ALL modes with comparison table\n");
    printf("  --ai        Which side AI plays (default: O)\n");
}

/* ---- main ---- */
int main(int argc, char *argv[]) {
    int  mode       = 0;
    int  depth      = 4;
    char ai_player  = PLAYER_O;
    const char *bench_path     = NULL;
    const char *bench_all_path = NULL;
    int  do_play = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
            if (mode < 0 || mode > 5) {
                fprintf(stderr, "Error: mode must be 0..5\n"); return 1;
            }
        } else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            depth = atoi(argv[++i]);
            if (depth < 1 || depth > 20) {
                fprintf(stderr, "Error: depth must be 1..20\n"); return 1;
            }
        } else if (strcmp(argv[i], "--ai") == 0 && i + 1 < argc) {
            i++;
            if (argv[i][0] == 'X' || argv[i][0] == 'x') ai_player = PLAYER_X;
            else if (argv[i][0] == 'O' || argv[i][0] == 'o') ai_player = PLAYER_O;
            else { fprintf(stderr, "Error: --ai must be X or O\n"); return 1; }
        } else if (strcmp(argv[i], "--bench") == 0 && i + 1 < argc) {
            bench_path = argv[++i];
        } else if (strcmp(argv[i], "--bench-all") == 0 && i + 1 < argc) {
            bench_all_path = argv[++i];
        } else if (strcmp(argv[i], "--play") == 0) {
            do_play = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]); return 0;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            print_usage(argv[0]); return 1;
        }
    }

    /* Initialize Zobrist keys (harmless if not used). */
    mode3_init();
    mode5_init();

    /* --bench-all mode */
    if (bench_all_path) {
        BoardEntry boards[MAX_BOARDS];
        int n = load_boards(bench_all_path, boards);
        if (n <= 0) {
            fprintf(stderr, "Error: failed to load boards from '%s'\n", bench_all_path);
            return 1;
        }
        run_bench_all(boards, n, depth);
        return 0;
    }

    /* --bench mode */
    if (bench_path) {
        BoardEntry boards[MAX_BOARDS];
        int n = load_boards(bench_path, boards);
        if (n <= 0) {
            fprintf(stderr, "Error: failed to load boards from '%s'\n", bench_path);
            return 1;
        }
        run_benchmark(boards, n, mode, depth);
        return 0;
    }

    /* --play mode */
    if (do_play) {
        run_interactive(mode, depth, ai_player);
        return 0;
    }

    print_usage(argv[0]);
    return 0;
}
