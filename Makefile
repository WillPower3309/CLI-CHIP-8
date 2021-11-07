CC=gcc
CFLAGS=-Wall -pedantic -std=gnu99
LDFLAGS=-lncursesw
OBJECTS=chip8.c main.c

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o c8 $(LDFLAGS)
