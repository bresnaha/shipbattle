CC = clang
CFLAGS = -g
all: server

clean:
	rm -f server

client: client.c ui.c ui.h
	$(CC) $(CFLAGS) -pthread -o server server.c
