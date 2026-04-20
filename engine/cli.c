#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "board.h"
#include "moves.h"
#include "rules.h"
#include "make_move.h"

#define INPUT_BUF 128

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[--n] = '\0';
    }
}

static void print_help(void) {
    printf("Commands:\n");
    printf("  place <col>          Drop your piece into column <col> (0-%d)\n", COLS - 1);
    printf("  remove <row> <col>   Remove a piece at (row, col). Uses your one removal.\n");
    printf("  help                 Show this help text\n");
    printf("  quit                 Exit the game\n");
}

static void print_banner(void) {
    printf("========================================\n");
    printf("  Modified Connect 4\n");
    printf("  Each player has ONE removal per game.\n");
    printf("========================================\n");
}

static int select_mode(void) {
    char line[INPUT_BUF];
    while (1) {
        printf("\nSelect mode:\n");
        printf("  1) Human vs Human\n");
        printf("  2) Human vs Bot\n");
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) exit(0);
        trim_newline(line);

        if (strcmp(line, "1") == 0) return 0;

        if (strcmp(line, "2") == 0) {
            while (1) {
                printf("Which side plays the bot? (x/o)\n> ");
                fflush(stdout);
                if (!fgets(line, sizeof(line), stdin)) exit(0);
                trim_newline(line);
                if (line[0] == 'x' || line[0] == 'X') return PLAYER_X;
                if (line[0] == 'o' || line[0] == 'O') return PLAYER_O;
                printf("Please enter 'x' or 'o'.\n");
            }
        }

        printf("Please enter 1 or 2.\n");
    }
}

static int run_bot_turn(GameState *state) {
    int move_type = -1;
    int row = -1;
    int column = -1;
    int player = state->current_player;

    printf("\n[Bot %s] thinking...\n", player_name(player));
    make_move(state, player, &move_type, &row, &column);

    Move m;
    if (move_type == MOVE_PLACEMENT) {
        m.type = MOVE_PLACEMENT;
        m.column = column;
        m.row = 0;
        m.col = 0;
    } else if (move_type == MOVE_REMOVAL) {
        m.type = MOVE_REMOVAL;
        m.column = 0;
        m.row = row;
        m.col = column;
    } else {
        printf("Bot produced no move (make_move is not implemented yet).\n");
        printf("  move_type=%d row=%d column=%d\n", move_type, row, column);
        return 0;
    }

    if (!is_legal_move(state, m)) {
        printf("Bot produced an illegal move: move_type=%d row=%d column=%d\n",
               move_type, row, column);
        return 0;
    }

    if (m.type == MOVE_PLACEMENT) {
        printf("Bot %s plays: place %d\n", player_name(player), m.column);
    } else {
        printf("Bot %s plays: remove %d %d\n", player_name(player), m.row, m.col);
    }
    apply_move(state, m);
    return 1;
}

static int parse_and_run_command(GameState *state, const char *line) {
    char cmd[32];
    int a = -1, b = -1;
    int n = sscanf(line, "%31s %d %d", cmd, &a, &b);

    if (n < 1) {
        printf("Empty input. Type 'help' for commands.\n");
        return 0;
    }

    for (char *p = cmd; *p; p++) *p = (char)tolower((unsigned char)*p);

    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        printf("Goodbye.\n");
        exit(0);
    }

    if (strcmp(cmd, "help") == 0) {
        print_help();
        return 0;
    }

    if (strcmp(cmd, "place") == 0) {
        if (n < 2) {
            printf("Usage: place <col>\n");
            return 0;
        }
        Move m = { .type = MOVE_PLACEMENT, .column = a, .row = 0, .col = 0 };
        if (!is_legal_move(state, m)) {
            printf("Illegal placement: column %d is out of range or full.\n", a);
            return 0;
        }
        apply_move(state, m);
        return 1;
    }

    if (strcmp(cmd, "remove") == 0) {
        if (n < 3) {
            printf("Usage: remove <row> <col>\n");
            return 0;
        }
        Move m = { .type = MOVE_REMOVAL, .column = 0, .row = a, .col = b };
        if (!is_legal_move(state, m)) {
            if (state->removals_remaining[state->current_player] <= 0) {
                printf("Illegal removal: player %s has no removals left.\n",
                       player_name(state->current_player));
            } else {
                printf("Illegal removal: (%d, %d) is out of range or empty.\n", a, b);
            }
            return 0;
        }
        apply_move(state, m);
        return 1;
    }

    printf("Unknown command '%s'. Type 'help' for commands.\n", cmd);
    return 0;
}

int main(void) {
    GameState state;
    init_game(&state);

    print_banner();
    int bot_player = select_mode();

    if (bot_player == 0) {
        printf("\nMode: Human vs Human\n");
    } else {
        printf("\nMode: Human vs Bot (bot plays %s)\n", player_name(bot_player));
    }
    print_help();
    display_board(&state);

    char line[INPUT_BUF];

    while (!state.is_terminal) {
        if (bot_player != 0 && state.current_player == bot_player) {
            if (!run_bot_turn(&state)) {
                printf("Exiting: bot could not produce a legal move.\n");
                return 1;
            }
            display_board(&state);
            continue;
        }

        printf("\n[Player %s] > ", player_name(state.current_player));
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\nEOF received. Exiting.\n");
            break;
        }
        trim_newline(line);

        if (parse_and_run_command(&state, line)) {
            display_board(&state);
        }
    }

    printf("\n========================================\n");
    if (state.winner == PLAYER_X) {
        printf("  Game over: Player X wins!\n");
    } else if (state.winner == PLAYER_O) {
        printf("  Game over: Player O wins!\n");
    } else {
        printf("  Game over: Draw.\n");
    }
    printf("========================================\n");

    return 0;
}
