CC=gcc

all:
	$(CC) -g -o mouse.o mouse.c
clean:
	-rm -f mouse.o
