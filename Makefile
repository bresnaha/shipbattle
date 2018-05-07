CC = clang
CFLAGS = -g

all: captain

clean:
	rm -f captain

captain: captain.c ui.c ui.h
	$(CC) $(CFLAGS) -pthread -o captain captain.c ui.c -lncurses
