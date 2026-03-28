#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "bot.h"
#include "board.h"
#include "board_loader.h"

#define MAX_OPPONENTS 32

/* Play one game. bot_a is player 0, bot_b is player 1.
 * Returns CELL_P0, CELL_P1, or WINNER_NONE (draw). */
int play_one_game(const Bot *bot_a, const Bot *bot_b, const GameState *initial);

/* Run the full experiment suite.
 * student_bot: the bot being tested.
 * opponents/num_opponents: array of opponent bots.
 * board_states/board_names/num_boards: starting positions.
 */
void run_experiment(const Bot *student_bot,
                    const Bot *opponents[], int num_opponents,
                    const GameState *board_states,
                    const char board_names[][MAX_STATE_NAME],
                    int num_boards);

#endif
