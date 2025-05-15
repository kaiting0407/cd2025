CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: parser

parser: main.c
	$(CC) $(CFLAGS) -o parser main.c

clean:
	rm -f parser *.o

.PHONY: all clean 