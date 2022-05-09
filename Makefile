CC=gcc
CPP=g++
CFLAGS=-g -Wall

BIN=blackjackserver player

all: $(BIN)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

%: %.cpp
	$(CPP) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -rf player_*.log server.sock $(BIN)
