/*
 * CLI for Connect 4 with Removals.
 * Main entry point: interactive play, watch mode, experiment runner.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "moves.h"
#include "rules.h"
#include "bot.h"
#include "experiment.h"
#include "board_loader.h"
#include "../evaluators/evaluators.h"
#include "../opponents/opponents.h"

#define MAX_BOTS 32

typedef struct {
    const char *name;
    EvaluateFn fn;
} BotEntry;

/* ============================================================ */
/*                     Evaluator Selection                       */
/* ============================================================ */

static int select_bot(const char *prompt, BotEntry *list, int count,
                      const char **name_out, EvaluateFn *fn_out) {
    int i, choice;
    printf("\n%s:\n", prompt);
    for (i = 0; i < count; i++)
        printf("  %d. %s\n", i + 1, list[i].name);
    printf("  0. Cancel\n");

    printf("\nEnter choice: ");
    if (scanf("%d", &choice) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input.\n");
        return 0;
    }
    { int c; while ((c = getchar()) != '\n' && c != EOF); }

    if (choice == 0) return 0;
    if (choice < 1 || choice > count) {
        printf("Invalid choice.\n");
        return 0;
    }

    *name_out = list[choice - 1].name;
    *fn_out = list[choice - 1].fn;
    return 1;
}

/* ============================================================ */
/*                     Interactive Play                          */
/* ============================================================ */

static void play_interactive_game(Bot *bot) {
    GameState state;
    gamestate_init(&state);

    printf("\nYou are player O. Playing against %s.\n", bot->name);
    printf("Commands:\n");
    printf("  place <column>     - Drop a piece in column (0-6)\n");
    printf("  remove <row> <col> - Remove piece at (row, col)\n");
    printf("  quit               - Exit game\n\n");
    gamestate_display(&state);
    printf("\n");

    while (!state.is_terminal) {
        Move legal_moves[MAX_LEGAL_MOVES];
        int num_moves = get_legal_moves(&state, legal_moves);
        if (num_moves == 0) {
            printf("No legal moves available.\n");
            break;
        }

        if (state.current_player == CELL_P0) {
            /* Human turn */
            char cmd[64];
            int valid = 0;

            while (!valid) {
                printf("Your move: ");
                if (!fgets(cmd, sizeof(cmd), stdin)) return;

                cmd[strcspn(cmd, "\n")] = '\0';

                if (strcmp(cmd, "quit") == 0) return;

                if (strncmp(cmd, "place ", 6) == 0) {
                    int col;
                    if (sscanf(cmd + 6, "%d", &col) == 1) {
                        Move m = move_placement(col);
                        int i, found = 0;
                        for (i = 0; i < num_moves; i++) {
                            if (move_equals(legal_moves[i], m)) { found = 1; break; }
                        }
                        if (found) {
                            state = apply_move(&state, m);
                            valid = 1;
                        } else {
                            printf("Illegal move. Try again.\n");
                        }
                    } else {
                        printf("Invalid format. Try: place <col>\n");
                    }
                } else if (strncmp(cmd, "remove ", 7) == 0) {
                    int row, col;
                    if (sscanf(cmd + 7, "%d %d", &row, &col) == 2) {
                        Move m = move_removal(row, col);
                        int i, found = 0;
                        for (i = 0; i < num_moves; i++) {
                            if (move_equals(legal_moves[i], m)) { found = 1; break; }
                        }
                        if (found) {
                            state = apply_move(&state, m);
                            valid = 1;
                        } else {
                            printf("Illegal move. Try again.\n");
                        }
                    } else {
                        printf("Invalid format. Try: remove <row> <col>\n");
                    }
                } else {
                    printf("Unknown command. Use: place <col>, remove <row> <col>, or quit\n");
                }
            }
        } else {
            /* Bot turn */
            Move move;
            char move_str[64];
            if (!bot_choose_move(bot, &state, &move)) {
                printf("Bot has no legal moves.\n");
                break;
            }
            move_to_string(move, move_str);
            printf("\n%s plays: %s\n", bot->name, move_str);
            state = apply_move(&state, move);
        }

        printf("\n");
        gamestate_display(&state);
        printf("\n");
    }

    /* Game over */
    printf("\n==================================================\n");
    if (state.winner != WINNER_NONE) {
        const char *winner_label = (state.winner == CELL_P0) ? "You" : bot->name;
        printf("Game over! Player %c (%s) wins!\n",
               state.winner == CELL_P0 ? 'O' : 'X', winner_label);
    } else {
        printf("Game over! Draw (board full).\n");
    }
    printf("==================================================\n");
}

/* ============================================================ */
/*                     Watch Mode                                */
/* ============================================================ */

static void watch_game(Bot *bot1, Bot *bot2, const GameState *initial_state) {
    GameState state;
    int move_count = 0;

    state = *initial_state;
    printf("\n%s (O) vs %s (X)\n", bot1->name, bot2->name);
    printf("==================================================\n");

    while (!state.is_terminal) {
        Move legal_moves[MAX_LEGAL_MOVES];
        int num_moves = get_legal_moves(&state, legal_moves);
        if (num_moves == 0) break;

        Move move;
        const char *bot_name;
        int found;

        if (state.current_player == CELL_P0) {
            found = bot_choose_move(bot1, &state, &move);
            bot_name = bot1->name;
        } else {
            found = bot_choose_move(bot2, &state, &move);
            bot_name = bot2->name;
        }

        if (!found) break;

        {
            char move_str[64];
            move_to_string(move, move_str);
            printf("\nMove %d: %s plays %s\n", move_count + 1, bot_name, move_str);
        }

        state = apply_move(&state, move);
        move_count++;

        printf("\n");
        gamestate_display(&state);
    }

    printf("\n==================================================\n");
    if (state.winner != WINNER_NONE) {
        const char *winner_name = (state.winner == CELL_P0) ? bot1->name : bot2->name;
        printf("Game over! Player %c (%s) wins in %d moves!\n",
               state.winner == CELL_P0 ? 'O' : 'X', winner_name, move_count);
    } else {
        printf("Game over! Draw in %d moves.\n", move_count);
    }
    printf("==================================================\n");
}

/* ============================================================ */
/*                     Main Menu                                 */
/* ============================================================ */

int main(void) {
    int num_evals = (int)NUM_EVALUATORS;
    int num_opponents = (int)NUM_OPPONENTS;
    int num_all_bots = num_evals + num_opponents;
    int i;

    /* Build evaluator list (for play/experiment selection) */
    BotEntry *eval_list = (BotEntry *)malloc(sizeof(BotEntry) * num_evals);
    for (i = 0; i < num_evals; i++) {
        eval_list[i].name = evaluator_bots[i].name;
        eval_list[i].fn = evaluator_bots[i].evaluate;
    }

    /* Build combined list for watch mode (evaluators + opponents) */
    BotEntry *all_bots = (BotEntry *)malloc(sizeof(BotEntry) * num_all_bots);
    for (i = 0; i < num_evals; i++) {
        all_bots[i].name = evaluator_bots[i].name;
        all_bots[i].fn = evaluator_bots[i].evaluate;
    }
    for (i = 0; i < num_opponents; i++) {
        all_bots[num_evals + i].name = opponent_bots[i].name;
        all_bots[num_evals + i].fn = opponent_bots[i].evaluate;
    }

    while (1) {
        int choice;

        printf("\n==================================================\n");
        printf("Connect 4 with Removals - AI Heuristic Design\n");
        printf("==================================================\n");
        printf("1. Play against a bot\n");
        printf("2. Watch two bots play\n");
        printf("3. Run experiment\n");
        printf("4. Quit\n\n");

        printf("Choose option (1-4): ");
        if (scanf("%d", &choice) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input.\n");
            continue;
        }
        { int c; while ((c = getchar()) != '\n' && c != EOF); }

        if (choice == 1) {
            const char *name;
            EvaluateFn fn;
            if (select_bot("Select a bot to play against", eval_list, num_evals,
                           &name, &fn)) {
                Bot bot = bot_create(fn, DEFAULT_DEPTH, name);
                play_interactive_game(&bot);
            }

        } else if (choice == 2) {
            const char *name1, *name2;
            EvaluateFn fn1, fn2;
            printf("\nSelect first bot:");
            if (select_bot("Available bots", all_bots, num_all_bots, &name1, &fn1)) {
                printf("Select second bot:");
                if (select_bot("Available bots", all_bots, num_all_bots, &name2, &fn2)) {
                    /* Load board states and let user pick a starting position */
                    GameState board_states[MAX_BOARD_STATES];
                    char board_names[MAX_BOARD_STATES][MAX_STATE_NAME];
                    int num_boards = load_board_states("board_states",
                                                       board_states, board_names,
                                                       MAX_BOARD_STATES);
                    GameState initial;
                    gamestate_init(&initial); /* default: empty board */

                    printf("\nStarting board state:\n");
                    printf("  0. Empty board (default start)\n");
                    for (i = 0; i < num_boards; i++)
                        printf("  %d. %s\n", i + 1, board_names[i]);
                    printf("\nEnter choice: ");

                    int bs_choice;
                    if (scanf("%d", &bs_choice) == 1 &&
                        bs_choice >= 1 && bs_choice <= num_boards) {
                        initial = board_states[bs_choice - 1];
                        printf("Starting from: %s\n", board_names[bs_choice - 1]);
                    } else {
                        printf("Starting from empty board.\n");
                    }
                    { int c; while ((c = getchar()) != '\n' && c != EOF); }

                    Bot bot1 = bot_create(fn1, DEFAULT_DEPTH, name1);
                    Bot bot2 = bot_create(fn2, DEFAULT_DEPTH, name2);
                    watch_game(&bot1, &bot2, &initial);
                }
            }

        } else if (choice == 3) {
            const char *name;
            EvaluateFn fn;
            if (select_bot("Select a bot to test", eval_list, num_evals,
                           &name, &fn)) {
                /* Load board states */
                GameState board_states[MAX_BOARD_STATES];
                char board_names[MAX_BOARD_STATES][MAX_STATE_NAME];
                int num_boards = load_board_states("board_states",
                                                    board_states, board_names,
                                                    MAX_BOARD_STATES);

                if (num_boards == 0) {
                    printf("No board states found in board_states/ folder.\n");
                    continue;
                }

                {
                    const Bot *opp_ptrs[MAX_OPPONENTS];
                    int oi;
                    for (oi = 0; oi < num_opponents; oi++)
                        opp_ptrs[oi] = &opponent_bots[oi];

                    Bot student_bot = bot_create(fn, DEFAULT_DEPTH, name);
                    run_experiment(&student_bot, opp_ptrs, num_opponents,
                                  board_states,
                                  (const char (*)[MAX_STATE_NAME])board_names,
                                  num_boards);
                }
            }

        } else if (choice == 4) {
            printf("Goodbye!\n");
            free(eval_list);
            free(all_bots);
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }

    return 0;
}
