.PHONY: all clean

all:
	gcc -Wall -Wextra -O2 -Wno-unused-parameter \
	  -o connect4game \
	  engine/board.c engine/moves.c engine/rules.c engine/search.c \
	  engine/bot.c engine/experiment.c engine/board_loader.c \
	  engine/cJSON.c engine/cli.c \
	  heuristics.c evaluators/*.c opponents/*.c -lm

clean:
	rm -f connect4game connect4game.exe *.o
