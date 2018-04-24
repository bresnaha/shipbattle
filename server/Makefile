CC = clang
CFLAGS = -g
all: server

clean:
	rm -f server

client: client.c ui.c ui.h
	$(CC) $(CFLAGS) -o server server.c -pthread
