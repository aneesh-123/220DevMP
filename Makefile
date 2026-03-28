# Connect 4 with Removals — Makefile
# Override compiler: make CC=clang  or  make CC=x86_64-w64-mingw32-gcc
CC = gcc
CFLAGS = -Wall -Wextra -O2 -Wno-unused-parameter

# Source files
ENGINE_SRC = engine/board.c engine/moves.c engine/rules.c engine/search.c \
             engine/bot.c engine/experiment.c engine/board_loader.c \
             engine/integrity.c engine/cJSON.c engine/cli.c

HEURISTICS_SRC = heuristics.c

EVALUATOR_SRC = $(wildcard evaluators/*.c)

OPPONENT_SRC = $(wildcard opponents/*.c)

ALL_SRC = $(ENGINE_SRC) $(HEURISTICS_SRC) $(EVALUATOR_SRC) $(OPPONENT_SRC)

TARGET = connect4game

# ============================================================
# Build targets
# ============================================================

all: $(TARGET)

$(TARGET): $(ALL_SRC)
	$(CC) $(CFLAGS) -o $@ $(ALL_SRC) -lm

# ============================================================
# Integrity checksums
# ============================================================

checksums:
	@echo "Generating .engine_checksums..."
	@rm -f .engine_checksums
	@for f in engine/board.h engine/board.c engine/moves.h engine/moves.c \
	          engine/rules.h engine/rules.c engine/search.h engine/search.c \
	          engine/bot.h engine/bot.c engine/experiment.h engine/experiment.c \
	          engine/board_loader.h engine/board_loader.c \
	          engine/integrity.h engine/integrity.c \
	          engine/cJSON.h engine/cJSON.c engine/cli.c; do \
		if [ -f "$$f" ]; then \
			hash=$$(sha256sum "$$f" | cut -d' ' -f1); \
			echo "$$hash  $$f" >> .engine_checksums; \
		fi; \
	done
	@echo "Done. Checksums written to .engine_checksums"

# ============================================================
# Convenience targets
# ============================================================

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all clean run checksums
