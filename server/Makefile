CC = clang
CFLAGS = -g
all: lobby

clean:
	rm -f lobby server
	
server: server.c server.h
	$(CC) $(CFLAGS) -o server server.c -pthread

lobby: lobby.c server.c server.h
	$(CC) $(CFLAGS) -o lobby lobby.c server.c -pthread

test: 
	$(CC) $(CFLAGS) -o test test.c test_utilities.c server.c -pthread
