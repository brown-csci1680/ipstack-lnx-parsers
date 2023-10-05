CC=gcc
CFLAGS=-Wall -g
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

all: example

objs: $(SRC)
	$(CC) $(CFLAGS) -c $^


example: $(OBJS)
	$(CC) $(CFLAGS) -o example $^


clean:
	rm -fv example *.o
