CC=gcc
HEADERS = $(wildcard *.h)
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

all: tp1

tp1: $(OBJS)
	$(CC) $(OBJS) -o tp1

%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@


clean:
	rm -f *.o tp1
	