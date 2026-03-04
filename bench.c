#include "bench.h"
#include "ai.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- Cross-platform timer (same as main.c) ---- */
static double bench_time_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6;
#endif
}

/* ---- Board file parser ---- */

/* Extract just the filename from a full path. */
static const char *basename_of(const char *path) {
    const char *p = path;
    const char *last = path;
    while (*p) {
        if (*p == '/' || *p == '\\') last = p + 1;
        p++;
    }
    return last;
}

int load_board(const char *filepath, BoardEntry *out) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s'\n", filepath);
        return -1;
    }

    /* Store the filename. */
    const char *fname = basename_of(filepath);
    strncpy(out->name, fname, sizeof(out->name) - 1);
    out->name[sizeof(out->name) - 1] = '\0';

    /* Defaults. */
    GameState *s = &out->state;
    memset(s->board, EMPTY, sizeof(s->board));
    s->current_player = PLAYER_X;
    s->removals_left_X = MAX_REMOVALS;
    s->removals_left_O = MAX_REMOVALS;

    char line[256];
    int rows_read = 0;

    while (fgets(line, sizeof(line), f)) {
        /* Skip empty lines. */
        if (line[0] == '\n' || line[0] == '\r') continue;

        /* Check for metadata lines. */
        if (strncmp(line, "TURN", 4) == 0) {
            /* Parse: TURN X  or  TURN O */
            char player = 0;
            if (sscanf(line, "TURN %c", &player) == 1) {
                if (player == 'X' || player == 'x')
                    s->current_player = PLAYER_X;
                else if (player == 'O' || player == 'o')
                    s->current_player = PLAYER_O;
                else {
                    fprintf(stderr, "Error in '%s': TURN must be X or O\n",
                            filepath);
                    fclose(f);
                    return -1;
                }
            }
            continue;
        }

        if (strncmp(line, "REM", 3) == 0) {
            /* Parse: REM X=2 O=1 */
            int rx = MAX_REMOVALS, ro = MAX_REMOVALS;
            if (sscanf(line, "REM X=%d O=%d", &rx, &ro) == 2) {
                s->removals_left_X = rx;
                s->removals_left_O = ro;
            }
            continue;
        }

        /* Board row line: file gives TOP row first (row 5), bottom last (row 0).
           So the first board line maps to internal row (ROWS - 1 - rows_read). */
        if (rows_read < ROWS) {
            int internal_row = ROWS - 1 - rows_read;
            int col = 0;
            for (int i = 0; line[i] != '\0' && col < COLS; i++) {
                char ch = line[i];
                if (ch == '.' || ch == 'X' || ch == 'O') {
                    s->board[internal_row][col] = ch;
                    col++;
                }
                /* Skip spaces and other characters. */
            }
            if (col != COLS) {
                fprintf(stderr, "Error in '%s': row %d has %d cells (expected %d)\n",
                        filepath, rows_read, col, COLS);
                fclose(f);
                return -1;
            }
            rows_read++;
        }
    }

    fclose(f);

    if (rows_read != ROWS) {
        fprintf(stderr, "Error in '%s': read %d board rows (expected %d)\n",
                filepath, rows_read, ROWS);
        return -1;
    }

    return 0;
}

/* ---- Directory listing (portable) ---- */

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

int load_boards_from_dir(const char *dirpath, BoardEntry *boards_out) {
    /* Strip trailing slashes/backslashes for clean path joining. */
    char dir_clean[512];
    strncpy(dir_clean, dirpath, sizeof(dir_clean) - 1);
    dir_clean[sizeof(dir_clean) - 1] = '\0';
    size_t dlen = strlen(dir_clean);
    while (dlen > 0 && (dir_clean[dlen-1] == '/' || dir_clean[dlen-1] == '\\'))
        dir_clean[--dlen] = '\0';

    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\board*.txt", dir_clean);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        snprintf(pattern, sizeof(pattern), "%s/board*.txt", dir_clean);
        h = FindFirstFileA(pattern, &fd);
        if (h == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "Error: no board*.txt files in '%s'\n", dirpath);
            return -1;
        }
    }

    int count = 0;
    do {
        if (count >= MAX_BOARDS) {
            fprintf(stderr, "Warning: max %d boards; ignoring extras.\n",
                    MAX_BOARDS);
            break;
        }
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_clean, fd.cFileName);
        if (load_board(fullpath, &boards_out[count]) == 0)
            count++;
    } while (FindNextFileA(h, &fd));

    FindClose(h);

    if (count == 0) {
        fprintf(stderr, "Error: loaded 0 boards from '%s'\n", dirpath);
        return -1;
    }
    return count;
}

#else
/* POSIX */
#include <dirent.h>

int load_boards_from_dir(const char *dirpath, BoardEntry *boards_out) {
    DIR *d = opendir(dirpath);
    if (!d) {
        fprintf(stderr, "Error: cannot open directory '%s'\n", dirpath);
        return -1;
    }

    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        /* Match files starting with "board" and ending with ".txt". */
        if (strncmp(ent->d_name, "board", 5) != 0) continue;
        size_t len = strlen(ent->d_name);
        if (len < 9 || strcmp(ent->d_name + len - 4, ".txt") != 0) continue;

        if (count >= MAX_BOARDS) {
            fprintf(stderr, "Warning: max %d boards; ignoring extras.\n",
                    MAX_BOARDS);
            break;
        }
        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, ent->d_name);
        if (load_board(fullpath, &boards_out[count]) == 0)
            count++;
    }
    closedir(d);

    if (count == 0) {
        fprintf(stderr, "Error: loaded 0 boards from '%s'\n", dirpath);
        return -1;
    }
    return count;
}
#endif

/* ---- Benchmark runner ---- */

static const char *mode_label(int m) {
    switch (m) {
        case 0: return "minimax";
        case 1: return "alpha-beta";
        case 2: return "alpha-beta+TT";
        default: return "unknown";
    }
}

static const char *move_str(const Move *m, char *buf, int buflen) {
    if (m->type == MOVE_DROP)
        snprintf(buf, buflen, "DROP col=%d", m->col);
    else
        snprintf(buf, buflen, "REMOVE col=%d row=%d", m->col, m->row);
    return buf;
}

void run_benchmark(const BoardEntry *boards, int num_boards,
                   int depth, const int *modes, int num_modes,
                   const char *csv_path) {

    /* Accumulators for summary. */
    double   sum_time[3]  = {0, 0, 0};
    long long sum_nodes[3] = {0, 0, 0};
    long long sum_tt_probes = 0, sum_tt_hits = 0;
    int      run_count[3] = {0, 0, 0};

    /* CSV file handle. */
    FILE *csv = NULL;
    if (csv_path) {
        csv = fopen(csv_path, "w");
        if (!csv) {
            fprintf(stderr, "Warning: cannot open '%s' for CSV output.\n",
                    csv_path);
        } else {
            fprintf(csv, "board,mode,mode_name,depth,move,time_ms,nodes,"
                         "tt_probes,tt_hits\n");
        }
    }

    printf("============================================================\n");
    printf("  BENCHMARK — depth=%d, boards=%d, modes=", depth, num_boards);
    for (int i = 0; i < num_modes; i++)
        printf("%d%s", modes[i], i < num_modes - 1 ? "," : "");
    printf("\n");
    printf("============================================================\n\n");

    for (int b = 0; b < num_boards; b++) {
        printf("--- Board: %s ---\n", boards[b].name);
        print_board(&boards[b].state);

        printf("  Turn: %c | Removals X=%d O=%d\n\n",
               boards[b].state.current_player,
               boards[b].state.removals_left_X,
               boards[b].state.removals_left_O);

        for (int mi = 0; mi < num_modes; mi++) {
            int m = modes[mi];

            /* Clear TT before each run for mode 2 (fair comparison). */
            if (m == 2) tt_clear();

            AIStats stats;
            double t0 = bench_time_ms();
            Move best = ai_choose_move(&boards[b].state, m, depth, &stats);
            double t1 = bench_time_ms();
            double elapsed = t1 - t0;

            char movebuf[64];
            move_str(&best, movebuf, sizeof(movebuf));

            /* Per-board detail line. */
            printf("BOARD %-20s MODE %d DEPTH %d MOVE %-20s "
                   "TIME_MS %8.1f NODES %10lld",
                   boards[b].name, m, depth, movebuf, elapsed,
                   stats.nodes_expanded);
            if (m == 2) {
                printf(" TT_PROBES %8lld TT_HITS %8lld",
                       stats.tt_probes, stats.tt_hits);
            }
            printf("\n");

            /* CSV row. */
            if (csv) {
                fprintf(csv, "%s,%d,%s,%d,%s,%.1f,%lld,%lld,%lld\n",
                        boards[b].name, m, mode_label(m), depth, movebuf,
                        elapsed, stats.nodes_expanded,
                        stats.tt_probes, stats.tt_hits);
            }

            /* Accumulate for summary. */
            sum_time[m]  += elapsed;
            sum_nodes[m] += stats.nodes_expanded;
            run_count[m]++;
            if (m == 2) {
                sum_tt_probes += stats.tt_probes;
                sum_tt_hits   += stats.tt_hits;
            }
        }
        printf("\n");
    }

    /* ---- Summary table ---- */
    printf("============================================================\n");
    printf("  SUMMARY (averaged over %d board(s), depth=%d)\n",
           num_boards, depth);
    printf("============================================================\n");
    printf("  %-14s %10s %12s %12s %12s\n",
           "Mode", "avg_ms", "avg_nodes", "tt_hit_rate", "speedup");

    double baseline_nodes = 0;
    for (int mi = 0; mi < num_modes; mi++) {
        int m = modes[mi];
        if (run_count[m] == 0) continue;
        double avg_time  = sum_time[m]  / run_count[m];
        double avg_nodes = (double)sum_nodes[m] / run_count[m];

        /* Track mode 0 nodes as the baseline for speedup calculation. */
        if (m == 0) baseline_nodes = avg_nodes;

        char tt_rate[32] = "N/A";
        if (m == 2 && sum_tt_probes > 0) {
            snprintf(tt_rate, sizeof(tt_rate), "%.1f%%",
                     100.0 * sum_tt_hits / sum_tt_probes);
        }

        char speedup[32] = "1.00x";
        if (baseline_nodes > 0 && avg_nodes > 0) {
            snprintf(speedup, sizeof(speedup), "%.2fx",
                     baseline_nodes / avg_nodes);
        }

        printf("  Mode %-8d %10.1f %12.0f %12s %12s\n",
               m, avg_time, avg_nodes, tt_rate, speedup);
    }
    printf("\n");

    if (csv) {
        fclose(csv);
        printf("CSV results written to: %s\n", csv_path);
    }
}
