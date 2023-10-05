CC=gcc
CFLAGS=-Wall -g
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

all: demo

objs: $(SRC)
	$(CC) $(CFLAGS) -c $^


demo: $(OBJS)
	$(CC) $(CFLAGS) -o demo $^


clean:
	rm -fv demo *.o
