CC = gcc
CFLAGS = -Wall -pedantic -lsimavr

main: main.c
	$(CC) $(CFLAGS) -o $@ $<
