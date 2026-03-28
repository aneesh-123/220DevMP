#include "bot.h"

Bot bot_create(EvaluateFn evaluate, int depth, const char *name) {
    Bot bot;
    bot.evaluate = evaluate;
    bot.depth = depth;
    bot.name = name;
    return bot;
}

int bot_choose_move(const Bot *bot, const GameState *state, Move *move_out) {
    SearchResult result = search(state, bot->depth, bot->evaluate);
    if (result.has_move) {
        *move_out = result.best_move;
        return 1;
    }
    return 0;
}
