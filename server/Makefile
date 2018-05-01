CC = clang
CFLAGS = -g
all: server

clean:
	rm -f server

server: server.c server.h
	$(CC) $(CFLAGS) -pthread -o server server.c
