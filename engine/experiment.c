#include <stdio.h>
#include "experiment.h"
#include "rules.h"

int play_one_game(const Bot *bot_a, const Bot *bot_b, const GameState *initial) {
    GameState state;
    if (initial)
        gamestate_copy(initial, &state);
    else
        gamestate_init(&state);

    while (!state.is_terminal) {
        Move legal_moves[MAX_LEGAL_MOVES];
        int num_moves = get_legal_moves(&state, legal_moves);
        if (num_moves == 0) break;

        Move move;
        int found;
        if (state.current_player == CELL_P0)
            found = bot_choose_move(bot_a, &state, &move);
        else
            found = bot_choose_move(bot_b, &state, &move);

        if (!found) break;
        state = apply_move(&state, move);
    }

    return state.winner;
}

void run_experiment(const Bot *student_bot,
                    const Bot *opponents[], int num_opponents,
                    const GameState *board_states,
                    const char board_names[][MAX_STATE_NAME],
                    int num_boards) {
    int total_wins = 0, total_losses = 0, total_draws = 0;
    int opp_wins[MAX_OPPONENTS], opp_losses[MAX_OPPONENTS], opp_draws[MAX_OPPONENTS];
    int oi, bi;
    int total_games = num_opponents * num_boards * 2;

    printf("\nTesting: %s\n", student_bot->name);
    printf("%d opponents x %d board states x 2 sides = %d games\n",
           num_opponents, num_boards, total_games);
    printf("============================================================\n");

    for (oi = 0; oi < num_opponents; oi++) {
        opp_wins[oi] = 0;
        opp_losses[oi] = 0;
        opp_draws[oi] = 0;

        printf("\n  vs %s:\n", opponents[oi]->name);

        for (bi = 0; bi < num_boards; bi++) {
            const GameState *bs = &board_states[bi];
            int side;
            const char *results[2] = { NULL, NULL };

            for (side = 0; side < 2; side++) {
                int winner;
                int student_won, opp_won;
                const char *side_str = (side == 0) ? "O" : "X";

                if (side == 0) {
                    /* Student goes first */
                    winner = play_one_game(student_bot, opponents[oi], bs);
                    student_won = (winner == CELL_P0);
                    opp_won = (winner == CELL_P1);
                } else {
                    /* Opponent goes first */
                    winner = play_one_game(opponents[oi], student_bot, bs);
                    student_won = (winner == CELL_P1);
                    opp_won = (winner == CELL_P0);
                }

                if (student_won) {
                    opp_wins[oi]++;
                    char buf[32];
                    snprintf(buf, sizeof(buf), "Win as %s", side_str);
                    results[side] = student_won ? "Win" : "Loss";
                    /* Print inline below */
                } else if (opp_won) {
                    opp_losses[oi]++;
                } else {
                    opp_draws[oi]++;
                }

                /* We'll print after both sides */
                if (student_won)
                    results[side] = side == 0 ? "Win as O" : "Win as X";
                else if (opp_won)
                    results[side] = side == 0 ? "Loss as O" : "Loss as X";
                else
                    results[side] = side == 0 ? "Draw as O" : "Draw as X";
            }

            printf("    %s: %s, %s\n", board_names[bi], results[0], results[1]);
        }

        total_wins += opp_wins[oi];
        total_losses += opp_losses[oi];
        total_draws += opp_draws[oi];

        {
            int opp_total = opp_wins[oi] + opp_losses[oi] + opp_draws[oi];
            int opp_pct = opp_total > 0 ? (100 * opp_wins[oi] / opp_total) : 0;
            printf("    Subtotal: W:%d L:%d D:%d (%d%% wins)\n",
                   opp_wins[oi], opp_losses[oi], opp_draws[oi], opp_pct);
        }
    }

    /* Final summary */
    printf("\n============================================================\n");
    printf("EXPERIMENT RESULTS: %s\n", student_bot->name);
    printf("============================================================\n");

    for (oi = 0; oi < num_opponents; oi++) {
        int opp_total = opp_wins[oi] + opp_losses[oi] + opp_draws[oi];
        int pct = opp_total > 0 ? (100 * opp_wins[oi] / opp_total) : 0;
        printf("  vs %-20s  W:%2d  L:%2d  D:%2d  (%d%%)\n",
               opponents[oi]->name, opp_wins[oi], opp_losses[oi], opp_draws[oi], pct);
    }

    printf("------------------------------------------------------------\n");
    {
        int grand_total = total_wins + total_losses + total_draws;
        int win_pct = grand_total > 0 ? (100 * total_wins / grand_total) : 0;
        printf("  %-23s  W:%2d  L:%2d  D:%2d  (%d%%)\n",
               "TOTAL", total_wins, total_losses, total_draws, win_pct);
    }
    printf("============================================================\n");
}
