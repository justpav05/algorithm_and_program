CC = gcc
CFLAGS = -fsanitize=address -fsanitize=undefined -g -Wall -Wextra -Iinclude
SRC = Lab11VarB.c
TARGET = Lab11

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

debug: all
	valgrind --leak-check=full ./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run debug clean
