CC=gcc

all:
	$(CC) -std=c99 -g -o mouse.o mouse.c
clean:
	-rm -f mouse.o
