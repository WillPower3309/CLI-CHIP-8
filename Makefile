CC=gcc
CFLAGS=-Wall -O2 -std=gnu99
LDFLAGS=-lncurses
OBJECTS=chip8.c main.c

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o c8 $(LDFLAGS)

