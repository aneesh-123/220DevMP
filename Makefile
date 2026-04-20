CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -Iengine
SRC     = engine/board.c engine/rules.c engine/make_move.c engine/cli.c
OBJ     = $(SRC:.c=.o)
TARGET  = connect4

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) $(TARGET).exe
