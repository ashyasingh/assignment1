
CC=gcc
CFLAGS=-I.
DEPS=
OBJS=finds.o
LIBS=
PROG=finds

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

all: $(PROG)

clean:
	rm -f *.o $(PROG)

