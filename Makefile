CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
INCLUDES = -Iinclude

CLIENT_SRC = client/main.cpp
SERVER_SRC = server/main.cpp

CLIENT_OBJ = client/main.o
SERVER_OBJ = server/main.o

ALL = client/echo-client server/echo-server

.PHONY: all clean

all: $(ALL)

client/main.o: $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

server/main.o: $(SERVER_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

client/echo-client: $(CLIENT_OBJ)
	$(CC) $^ -o $@

server/echo-server: $(SERVER_OBJ)
	$(CC) $^ -o $@

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(ALL)
